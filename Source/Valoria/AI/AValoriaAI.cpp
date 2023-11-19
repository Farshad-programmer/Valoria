// Fill out your copyright notice in the Description page of Project Settings.


#include "AValoriaAI.h"
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
	capitalCode = FMath::RandRange(1, 9999);
	enemyStatus = EAIStatus::neutral;
}

void AValoriaAI::BeginPlay()
{
	Super::BeginPlay();
	enemyStatus = EAIStatus::neutral;



	FTimerHandle AIStartStatusHandler;
	GetWorldTimerManager().SetTimer(AIStartStatusHandler, this, &AValoriaAI::InitialAIStatus, 0.2f, false);



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
				FName myTag = FName(*tag);
				Spawnedworker->Tags.Add(myTag);
				Spawnedworker->SetCapitalCode(capitalCode);
				baseUnit++;
			}
		}
	}

	FindAPlaceForMakingBarracksforAI();
}


void AValoriaAI::InitialAIStatus()
{
	enemyStatus = EAIStatus::neutral;
}

void AValoriaAI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
				FName myTag = FName(*tag);
				spawnedInfantry->Tags.Add(myTag);
				spawnedInfantry->SetCapitalCode(capitalCode);
				infantryNumber++;
				baseUnit++;
				UpdateAIUnits();
			}
		}
	}
	else
	{
		UpdateAIUnits();
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
			if (NewUnit && !NewUnit->ActorHasTag("Player"))
			{
				NewUnit->enemyStatus = enemyStatus;
			}
		}
	}
}

void AValoriaAI::AIMoveToBuilding()
{
	Spawnedworker->AIMoveToBuildingLocation(this);
}
