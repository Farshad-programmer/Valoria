// Fill out your copyright notice in the Description page of Project Settings.


#include "AValoriaAI.h"

#include "Kismet/GameplayStatics.h"
#include "Valoria/MapBorder/MapBorder.h"
#include "Kismet/KismetMathLibrary.h"
#include "Valoria/Buildings/CityCenter.h"
#include "Valoria/Characters/ValoriaWorker.h"
#include "Valoria/Buildings/Barracks.h"
#include "Valoria/Characters/ValoriaInfantry.h"

// Sets default values
AValoriaAI::AValoriaAI()
{
	PrimaryActorTick.bCanEverTick = true;
	tag = FString(TEXT("AI")) + FString::FromInt(FMath::RandRange(1,9999));
	capitalCode = FMath::RandRange(1,9999);
}

void AValoriaAI::BeginPlay()
{
	Super::BeginPlay();
	TArray<AActor*>mapBordersActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMapBorder::StaticClass(), mapBordersActors);

	GetWorldTimerManager().SetTimer(spawnSoldierTimerHandle,this,&AValoriaAI::SpawnSoldier,5.f,true);


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
					mapBorderRef->borderStatus = EBorderStatus::enemy;

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


void AValoriaAI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AValoriaAI::SpawnSoldier()
{
	if (bHasBarracks && baseUnit < 30)
	{
		if(valoriaInfantryClass && GetWorld())
		{
			AValoriaInfantry* spawnedInfantry = GetWorld()->SpawnActor<AValoriaInfantry>(valoriaInfantryClass, barracksLocation[0], FRotator(0.f));
			if (spawnedInfantry)
			{
				FName myTag = FName(*tag);
				spawnedInfantry->Tags.Add(myTag);
				spawnedInfantry->SetCapitalCode(capitalCode);
				baseUnit++;
			}
		}
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
		}
		if (Spawnedworker)
		{
			Spawnedworker->buildingRef = spawnedBarraks;
			Spawnedworker->SetCheckForStartWork(true);
			GetWorldTimerManager().SetTimer(moveAITimerhandler,this,&AValoriaAI::AIMoveToBuilding,3.f,false);
		}
	}
}

void AValoriaAI::AIMoveToBuilding()
{
	Spawnedworker->AIMoveToBuildingLocation(this);
}
