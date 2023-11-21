// Fill out your copyright notice in the Description page of Project Settings.


#include "AValoriaAI.h"

#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Valoria/MapBorder/MapBorder.h"
#include "Kismet/KismetMathLibrary.h"
#include "Valoria/Buildings/CityCenter.h"
#include "Valoria/Characters/ValoriaWorker.h"
#include "Valoria/Buildings/Barracks.h"
#include "Valoria/Characters/ValoriaInfantry.h"


// Guide
// AI workers will make barracks  in FindAPlaceForMakingBarracksforAI function
// barracks will spawn soldiers unit if in Building.cpp we set buildingOwner != EBuildingOwner::self in tick function





// Sets default values
AValoriaAI::AValoriaAI()
{
	PrimaryActorTick.bCanEverTick = true;
	tag = FString(TEXT("AI")) + FString::FromInt(FMath::RandRange(1, 9999));
	enemyStatus = EAIStatus::neutral;
}

void AValoriaAI::BeginPlay()
{
	Super::BeginPlay();
	capitalCode = FMath::RandRange(1, 9999);
	enemyStatus = EAIStatus::neutral;



	FTimerHandle AIStartStatusHandler;
	GetWorldTimerManager().SetTimer(AIStartStatusHandler, this, &AValoriaAI::InitialAIStatus, 0.2f, false);
}


void AValoriaAI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CheckingUnitNumberInBase();
	CheckWarState();
}


void AValoriaAI::InitialAIStatus()
{
	enemyStatus = EAIStatus::neutral;


	TArray<AActor*>mapBordersActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMapBorder::StaticClass(), mapBordersActors);

	GetWorldTimerManager().SetTimer(spawnSoldierTimerHandle, this, &AValoriaAI::SpawnSoldier, 5.f, true);


	bool bspawning = true;
	while (bspawning)
	{
		mapBorderRef = Cast<AMapBorder>(mapBordersActors[UKismetMathLibrary::RandomIntegerInRange(0, mapBordersActors.Num() - 1)]);
		if (mapBorderRef && mapBorderRef->borderStatus == EBorderStatus::ownerless)
		{
			bspawning = false;
			FVector cityCenterSpawnedLocation = mapBorderRef->GetActorLocation();
			cityCenterSpawnedLocation.Z -= 500.f;
			AIBaseCenterLocation = cityCenterSpawnedLocation;
			if (GetWorld() && cityCenterClass)
			{
				ACityCenter* SpawnedCityCenter = GetWorld()->SpawnActor<ACityCenter>(cityCenterClass, AIBaseCenterLocation, FRotator(0.f));
				if (SpawnedCityCenter)
				{
					SpawnedCityCenter->SetIsStarterCityCenter(true);
					SpawnedCityCenter->SetBuildingToStarterBuilding();
					SpawnedCityCenter->capitalName = capitalName;
					SpawnedCityCenter->SetCapitalNameTextRender();
					SpawnedCityCenter->SetBuildingOwner(EBuildingOwner::neutral);
					SpawnedCityCenter->valoriaAIRef = this;
					mapBorderRef->bBorderHasCityCenter = true;
					mapBorderRef->borderStatus = EBorderStatus::neutral;

					// this part will be implemented later
					/*if(enemyStatus == EEnemyStatus::enemy)
					{
						mapBorderRef->borderStatus = EBorderStatus::enemy;
					}
					else if(enemyStatus == EEnemyStatus::ally)
					{
						mapBorderRef->borderStatus = EBorderStatus::ally;
					}*/
				}
			}
		}
	}

	for (int i = 0; i < 3; ++i)
	{
		if (GetWorld() && valoriaWorkerClass)
		{
			AIBaseCenterLocation.X += 500.f;
			AIBaseCenterLocation.Y += 500.f;
			AIBaseCenterLocation.Z += 300.f;
			Spawnedworker = GetWorld()->SpawnActor<AValoriaWorker>(valoriaWorkerClass, AIBaseCenterLocation, FRotator(0.f));
			if (Spawnedworker)
			{
				Spawnedworker->unitAIOwner = this;
				FName myTag = FName(*tag);
				Spawnedworker->Tags.Add(myTag);
				Spawnedworker->SetCapitalCode(capitalCode);
				baseUnit++;
			}
		}
	}

	FindAPlaceForMakingBarracksforAI();
}


void AValoriaAI::CheckWarState()
{
	if (enemyStatus == EAIStatus::enemy)
	{
		bInWarState = true;
		if (enemiesCode.Num() > 0)
		{
			capitalToAttack = enemiesCode[0];
			if (bOrderUnitsToAttackBase)
			{
				FindNearestBaseToAttack();
			}
		}
	}
	else
	{
		bInWarState = false;
	}
}

void AValoriaAI::FindNearestBaseToAttack()
{
	TArray<class AActor*> enemyBases;
	if (GetWorld())
	{
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACityCenter::StaticClass(), enemyBases);
		for (auto enemyBase : enemyBases)
		{
			ACityCenter* castedEnemyBase = Cast<ACityCenter>(enemyBase);
			if (castedEnemyBase)
			{
				if (castedEnemyBase->ActorHasTag("PlayerBase"))
				{
					baseToAttack = castedEnemyBase;
					bBaseToAttackIdentified = true;
					bOrderUnitsToAttackBase = false;
					OrderSoldiersToAttack(1);
					break;
				}
			}
		}
	}
}

void AValoriaAI::OrderSoldiersToAttack(int32 enemyCapitalCodeToAttack)
{
	bool bOrderToAttackbase = false;
	attackCounter++;
	if (attackCounter % 3 == 0)
	{
		bOrderToAttackbase = true;
	}

	TArray<class AActor*> units;
	if (GetWorld())
	{
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AValoriaCharacter::StaticClass(), units);
		for (auto unit : units)
		{
			AValoriaCharacter* castedUnit = Cast<AValoriaCharacter>(unit);
			if (castedUnit && !castedUnit->bMustAttackBase && castedUnit->GetCapitalCode() == capitalCode && !castedUnit->ActorHasTag("Worker"))
			{
				castedUnit->bMustAttackBase = bOrderToAttackbase;
				castedUnit->MoveToLocation(baseToAttack->GetActorLocation(), false, baseToAttack, nullptr, false, nullptr, true);
				castedUnit->SetInWarState(true);
			}

		}
		unitInCapital = 0;
	}

}



void AValoriaAI::SpawnSoldier()
{
	if (bHasBarracks && baseUnit < 30)
	{
		if (valoriaInfantryClass && GetWorld())
		{
			AValoriaInfantry* spawnedInfantry = GetWorld()->SpawnActor<AValoriaInfantry>(valoriaInfantryClass, barracksLocation[0], FRotator(0.f));
			if (spawnedInfantry)
			{
				spawnedInfantry->unitAIOwner = this;
				FName myTag = FName(*tag);
				spawnedInfantry->Tags.Add(myTag);
				spawnedInfantry->SetCapitalCode(capitalCode);
				infantryNumber++;
				baseUnit++;
				unitInCapital++;
				spawnedInfantry->enemyStatus = enemyStatus;
				UpdateAIUnits();
			}
		}
	}
	else
	{
		UpdateAIUnits();
	}
}


void AValoriaAI::CheckingUnitNumberInBase()
{
	if (unitInCapital > 5)
	{
		bOrderUnitsToAttackBase = true;
	}
}

void AValoriaAI::FindAPlaceForMakingBarracksforAI()
{
	if (mapBorderRef == nullptr)return;

	bool randBool = UKismetMathLibrary::RandomBool();
	FVector location = mapBorderRef->GetActorLocation();
	location.Z = -630.f;
	if (randBool)
	{
		location.X += (UKismetMathLibrary::RandomFloatInRange(1500.f, 3000.f));
		location.Y += (UKismetMathLibrary::RandomFloatInRange(1500.f, 3000.f));
	}
	else
	{
		location.X += (UKismetMathLibrary::RandomFloatInRange(-1500.f, -3000.f));
		location.Y += (UKismetMathLibrary::RandomFloatInRange(-1500.f, -3000.f));
	}

	if (GetWorld() && barracksClass)
	{
		ABarracks* spawnedBarraks = GetWorld()->SpawnActor<ABarracks>(barracksClass, location, FRotator(0.f));
		if (spawnedBarraks && spawnedBarraks->GetLevel1Mesh())
		{
			spawnedBarraks->GetBuildingMesh()->SetStaticMesh(spawnedBarraks->GetLevel1Mesh());
			spawnedBarraks->GetBuildingMesh()->SetMaterial(0, spawnedBarraks->buildingMat);
			spawnedBarraks->SetBuildingOwner(EBuildingOwner::neutral);
			spawnedBarraks->valoriaAIRef = this;
		}
		if (Spawnedworker)
		{
			Spawnedworker->buildingRef = spawnedBarraks;
			Spawnedworker->SetCheckForStartWork(true);
			GetWorldTimerManager().SetTimer(moveAITimerhandler, this, &AValoriaAI::AIMoveToBuilding, 3.f, false);
		}
	}
}

void AValoriaAI::UpdateAIUnits()
{

	TArray<class AActor*> enemyUnits;;
	if (GetWorld())
	{
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AValoriaCharacter::StaticClass(), enemyUnits);
		for (AActor* unit : enemyUnits)
		{
			AValoriaCharacter* NewUnit = Cast<AValoriaCharacter>(unit);
			if (NewUnit && !NewUnit->ActorHasTag("Player") && NewUnit->GetCapitalCode() == capitalCode)
			{
				NewUnit->enemyStatus = enemyStatus;
				if (enemyStatus == EAIStatus::enemy)
				{
					NewUnit->enemyUnitCodeToAttack.Add(1); // 1 is for player so AI can attack player
				}
			}
		}
	}
}

void AValoriaAI::AIMoveToBuilding()
{
	Spawnedworker->AIMoveToBuildingLocation(this);
}



