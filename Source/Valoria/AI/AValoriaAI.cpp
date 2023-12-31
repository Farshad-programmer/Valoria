// Fill out your copyright notice in the Description page of Project Settings.


#include "AValoriaAI.h"

#include "Components/TextRenderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Valoria/MapBorder/MapBorder.h"
#include "Kismet/KismetMathLibrary.h"
#include "Valoria/ValoriaCam.h"
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

	FTimerHandle AIUpgradeTimerHandler;
	GetWorldTimerManager().SetTimer(AIUpgradeTimerHandler, this, &AValoriaAI::UpdateAIStuff, 3.f, true);
}
void AValoriaAI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CheckingUnitNumberInBase();
	CheckWarState();
}
void AValoriaAI::InitialAIStatus()
{
	// set AI status in start of the game 
	enemyStatus = EAIStatus::neutral;

	// spawn AI Initial soldiers in specific period of time 
	GetWorldTimerManager().SetTimer(spawnSoldierTimerHandle, this, &AValoriaAI::SpawnSoldier, 5.f, true);

	// spawn AI Initial buildings and units
	SpawnAIFirstCityCenter();
	SpawnAIFirstWorkers();
	FindAPlaceForMakingBarracksforAI();

	playerRef = Cast<AValoriaCam>(UGameplayStatics::GetPlayerPawn(this, 0));
}
void AValoriaAI::SpawnAIFirstCityCenter()
{
	TArray<AActor*>mapBordersActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMapBorder::StaticClass(), mapBordersActors);
	bool bspawning = true;
	while (bspawning)
	{
		mapBorderRef = Cast<AMapBorder>(mapBordersActors[UKismetMathLibrary::RandomIntegerInRange(0, mapBordersActors.Num() - 1)]);
		if (mapBorderRef && mapBorderRef->borderStatus == EBorderStatus::ownerless)
		{
			bspawning = false;
			FVector cityCenterSpawnedLocation = mapBorderRef->cityCenterLocation->GetComponentLocation();
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
				}
			}
		}
	}
}
void AValoriaAI::SpawnAIFirstWorkers()
{
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
				AIAllUnitNumber++;
				Spawnedworker->SetCapitalCode(capitalCode);
				baseUnit++;
			}
		}
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
				AIAllUnitNumber++;
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
	if (attackCounter % 3 == 0) // this is a temporarity logic that set which soldiers must  attack base and which must attack units
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
	FVector location = mapBorderRef->barracksLocation->GetComponentLocation();

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

void AValoriaAI::UpdateAIStuff()
{
	wood = (wood + woodUpgradeRate) * AIRank;
	stone = (stone + stoneUpgradeRate) * AIRank;
	gold = (gold + goldUpgradeRate) * AIRank;
	science = (science + scieneUpgradeRate) * AIRank;
}


void AValoriaAI::AIMoveToBuilding()
{
	Spawnedworker->AIMoveToBuildingLocation(this);
}

void AValoriaAI::ManageAIActions()
{
	int32 reqestedWood = 0;
	int32 reqestedGold = 0;
	int32 reqestedStone = 0;
	int32 reqestedScience = 0;
	if (AIAllUnitNumber > 10)
	{
		InspectPlayerResources();
		if (bNeedAction && playerRef)
		{
			if (bAskWood)
			{
				reqestedWood = wood;
			}
			if (bAskWood)
			{
				reqestedGold = gold;
			}
			if (bAskWood)
			{
				reqestedStone = stone;
			}
			if (bAskWood)
			{
				reqestedScience = science;
			}
			playerRef->HandleNeighborRequest(this, reqestedWood,reqestedGold,reqestedStone,reqestedScience );
			bAskWood = false;
			bAskStone = false;
			bAskGold = false;
			bAskScience = false;
			bNeedAction = false;
		}
	}
}

void AValoriaAI::InspectPlayerForces()
{
	// todo
}

void AValoriaAI::InspectPlayerResources()
{
	if (playerRef)
	{
		int32 playerWood = playerRef->GetWood();
		int32 playerGold = playerRef->GetGold();
		int32 playerStone = playerRef->GetStone();
		int32 playerScience = playerRef->GetScience();

		if (playerWood > wood * 2)
		{
			bAskWood = true;
			bNeedAction = true;
		}
		if (playerStone > stone * 2)
		{
			bAskStone = true;
			bNeedAction = true;
		}
		if (playerGold > gold * 2)
		{
			bAskGold = true;
			bNeedAction = true;
		}
		if (playerScience > science * 2)
		{
			bAskScience = true;
			bNeedAction = true;
		}
		if (playerWood <= wood * 2 && playerStone <= stone * 2 && playerGold <= gold * 2 && playerScience <= science * 2)
		{
			bAskWood = false;
			bAskStone = false;
			bAskGold = false;
			bAskScience = false;
			bNeedAction = false;
		}
	}
}



