// Fill out your copyright notice in the Description page of Project Settings.


#include "Building.h"

#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Valoria/ValoriaCam.h"
#include "Valoria/ValoriaCharacter.h"

// Sets default values
ABuilding::ABuilding()
{
	PrimaryActorTick.bCanEverTick = true;
	BuildingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildingMesh"));
	BuildingMesh->SetupAttachment(RootComponent);
	Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	Widget->SetupAttachment(BuildingMesh);
	WorkerPoint1 = CreateDefaultSubobject<USceneComponent>(TEXT("WorkerPoint1"));
	WorkerPoint1->SetupAttachment(BuildingMesh);
	WorkerPoint2 = CreateDefaultSubobject<USceneComponent>(TEXT("WorkerPoint2"));
	WorkerPoint2->SetupAttachment(BuildingMesh);
	WorkerPoint3 = CreateDefaultSubobject<USceneComponent>(TEXT("WorkerPoint3"));
	WorkerPoint3->SetupAttachment(BuildingMesh);

	if (WorkerPoint1 && WorkerPoint2 && WorkerPoint3)
	{
		WorkerPoint1->SetRelativeLocation(FVector(-310.f, 0.f, 0.f));
		WorkerPoint2->SetRelativeLocation(FVector(310.f, 0.f, 0.f));
		WorkerPoint3->SetRelativeLocation(FVector(0.f, -310.f, 0.f));
	}

	BuildingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
}

void ABuilding::BeginPlay()
{
	Super::BeginPlay();
	valoriaCam = Cast<AValoriaCam>(UGameplayStatics::GetPlayerPawn(this, 0));
	buildingWorkPoints.Emplace(WorkerPoint1->GetRelativeLocation());
	buildingWorkPoints.Emplace(WorkerPoint2->GetRelativeLocation());
	buildingWorkPoints.Emplace(WorkerPoint3->GetRelativeLocation());
	if (buildingGreenMat && BuildingMesh)
	{
		BuildingMesh->SetMaterial(0, buildingGreenMat);
	}
}

void ABuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bConstructionProgressStarted)
	{
		constructionCounter += constructionProgressSpeed * workerNumber * DeltaTime;
		if (constructionCounter >= 2000.f && constructionCounter <= 2100.f)
		{
			BuildingMesh->SetStaticMesh(level2Mesh);
		}
		if (constructionCounter >= constrcutionFinishValue)
		{
			BuildingMesh->SetStaticMesh(level3Mesh);
			bConstructionProgressStarted = false;
			if (buidlingWorkers.Num())
			{
				for (auto worker : buidlingWorkers)
				{
					worker->StopWorkAnimation();
					workerNumber = 0;
					buildingWorkPoints.Empty();
					bConstructionIsBuilt = true;
				}
			}

		}
	}
	else
	{

		if (bIsBuildingSpawned)
		{
			bCanPlaceBuilding = true;
			valoriaCam->SetIsPlacingBuidling(true);
			APlayerController* playerController = UGameplayStatics::GetPlayerController(this, 0);
			if (playerController)
			{
				bool bCourserHitSuccessful = playerController->GetHitResultUnderCursor(ECollisionChannel::ECC_Camera, true, Hit);
				if (bCourserHitSuccessful)
				{
					FVector loc = Hit.Location;
					loc.Z += 100.f;
					SetActorLocation(loc);
				}
			}
			if (valoriaCam)
			{
				if (valoriaCam->buildingRef == nullptr)
				{
					valoriaCam->buildingRef = this;
				}

			}
		}
		else
		{
			if (valoriaCam)
			{
				if (valoriaCam->GetWood() >= wood && valoriaCam->GetStone() >= stone && valoriaCam->GetGold() >= gold && valoriaCam->GetScience() >= science)
				{
					bCanPlaceBuilding = false;
					if (valoriaCam->buildingRef)
					{
						valoriaCam->SetIsPlacingBuidling(false);
						//valoriaCam->buildingRef = nullptr;
						BuildingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
						if (buildingMat)
						{
							BuildingMesh->SetMaterial(0, buildingMat);
						}
						if (level1Mesh && !bConstructionIsBuilt && constructionCounter <= 0.f)
						{
							BuildingMesh->SetStaticMesh(level1Mesh);
							if (bUpdatingNeeds)
							{
								bUpdatingNeeds = false;
								valoriaCam->UpdateGold(false, gold);
								valoriaCam->UpdateScience(false, science);
								valoriaCam->UpdateStone(false, stone);
								valoriaCam->UpdateWood(false, wood);
							}

						}
					}
				}

			}
		}
	}



}

