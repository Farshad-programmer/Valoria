// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAI.h"

#include "Kismet/GameplayStatics.h"
#include "Valoria/MapBorder/MapBorder.h"
#include "Kismet/KismetMathLibrary.h"
#include "Valoria/Buildings/CityCenter.h"
#include "Valoria/Characters/ValoriaWorker.h"


// Sets default values
AEnemyAI::AEnemyAI()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AEnemyAI::BeginPlay()
{
	Super::BeginPlay();
	TArray<AActor*>mapBordersActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AMapBorder::StaticClass(), mapBordersActors);

	bool bspawning = true;
	while (bspawning)
	{
		AMapBorder* mapBorderRef = Cast<AMapBorder>(mapBordersActors[UKismetMathLibrary::RandomIntegerInRange(0,mapBordersActors.Num() - 1)]);
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
					mapBorderRef->borderStatus = EBorderStatus::enemy;
					mapBorderRef->bBorderHasCityCenter = true;
				}
			}
		}
	}

	for (int i = 0; i < 3; ++i)
	{
		if(GetWorld() && valoriaWorkerClass)
		{
			AIBaseCenterLocation.X += 500.f;
			AIBaseCenterLocation.Y += 500.f;
			AIBaseCenterLocation.Z += 300.f;
			AValoriaWorker* Spawnedworker = GetWorld()->SpawnActor<AValoriaWorker>(valoriaWorkerClass,AIBaseCenterLocation, FRotator(0.f));
		}
	}
}

void AEnemyAI::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

