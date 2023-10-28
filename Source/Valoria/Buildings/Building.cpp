// Fill out your copyright notice in the Description page of Project Settings.


#include "Building.h"

#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Valoria/ValoriaCam.h"
#include "Valoria/ValoriaCharacter.h"
#include "Valoria/Characters/ValoriaWorker.h"
#include "Valoria/MapBorder/MapBorder.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ABuilding::ABuilding()
{
	PrimaryActorTick.bCanEverTick = true;
	BuildingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildingMesh"));
	BuildingMesh->SetupAttachment(RootComponent);
	Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	Widget->SetupAttachment(BuildingMesh);
	box = CreateDefaultSubobject<UBoxComponent>(TEXT("box"));
	box->SetupAttachment(BuildingMesh);
	flagStarterPoint = CreateDefaultSubobject<USceneComponent>(TEXT("WorkerPoint1"));
	flagStarterPoint->SetupAttachment(BuildingMesh);



	if (flagStarterPoint)
	{
		flagStarterPoint->SetRelativeLocation(FVector(-310.f, 0.f, 0.f));

	}

	BuildingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
}

void ABuilding::BeginPlay()
{
	Super::BeginPlay();
	valoriaCam = Cast<AValoriaCam>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (buildingGreenMat && BuildingMesh)
	{
		BuildingMesh->SetMaterial(0, buildingGreenMat);
	}
}



void ABuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(buildingWorkPointsIndex > 0)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Green, FString::Printf(TEXT("%s"),*GetName()));
	}
	


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
			if (BorderRef)
			{
				BorderRef->UpdateBorderOwner(EBorderStatus::self);
				BorderRef = nullptr;
			}

			bConstructionProgressStarted = false;
			if (buidlingWorkers.Num())
			{
				for (auto worker : buidlingWorkers)
				{
					worker->StopWorkAnimation();
					workerNumber = 0;
					buildingMaxWorker = 0;
					bConstructionIsBuilt = true;
				}
			}

		}
	}
	else
	{

		if (bIsBuildingSpawned)
		{
			CheckCanBuild();
			valoriaCam->SetIsPlacingBuidling(true);
			APlayerController* playerController = UGameplayStatics::GetPlayerController(this, 0);
			if (playerController)
			{
				bool bCourserHitSuccessful = playerController->GetHitResultUnderCursor(ECollisionChannel::ECC_Camera, true, Hit);
				if (bCourserHitSuccessful)
				{
					FVector loc = Hit.Location;
					FVector GridCellSize = FVector(100.0f, 100.0f, 100.0f);
					loc.X = FMath::FloorToInt(loc.X / GridCellSize.X) * GridCellSize.X;
					loc.Y = FMath::FloorToInt(loc.Y / GridCellSize.Y) * GridCellSize.Y;
					loc.Z += 100.f;
					SetActorLocation(loc);
					//FRotator rot = UKismetMathLibrary::MakeRotFromZX(Hit.ImpactPoint,GetActorForwardVector());
					//SetActorRotation(rot);
					ValidateBuildLocation(loc);

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
			if (valoriaCam && valoriaCam->buildingRef)
			{
				if (bBuildingIsAllowedToBeBuilt)
				{
					bBuildingIsAllowedToBeBuilt = false;
					if (valoriaCam->buildingRef)
					{
						valoriaCam->SetIsPlacingBuidling(false);
						BuildingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
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
							if (buildingMat)
							{
								BuildingMesh->SetMaterial(0, buildingMat);
							}
							if (BorderRef)
							{
								BorderRef->borderStatus = EBorderStatus::self;
								BorderRef->bBorderHasCityCenter = true;
							}
							if(valoriaCam->PlayerTemp)
							{
								valoriaCam->PlayerTemp->MoveToLocation(this->GetActorLocation(),true,this,nullptr);
								valoriaCam->PlayerTemp = nullptr;
							}
							
						}
					}
				}
				else
				{
					if (!bBuildingPlaced)
					{
						bBuildingPlaced = true;
						valoriaCam->buildingRef->SetBuildingIsAllowedToBeBuilt(false);
						if (valoriaCam->buildingRef && valoriaCam->GetIsPlacingBuilding())
						{
							valoriaCam->SetIsPlacingBuidling(false);
						}

					}
				}
			}
		}
	}
}


void ABuilding::ValidateBuildLocation(FVector loc)
{
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors);
	if (OverlappingActors.Num() > 0 || loc.Z > 120.f)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Orange, FString::FromInt(OverlappingActors.Num()));
		if (buildingRedMat && BorderRef)
		{
			if (BorderRef->bBorderHasCityCenter || BorderRef->borderStatus == EBorderStatus::enemy)
			{
				BuildingMesh->SetMaterial(0, buildingRedMat);
				bCanCheck = false;
				OverlappingActors.Empty();
			}
			else
			{
				if (buildingGreenMat)
				{
					if (bBuildingIsAllowedToBeBuilt)
					{
						BuildingMesh->SetMaterial(0, buildingGreenMat);
					}
					bCanCheck = true;
				}
			}

		}
		if (buildingType != EBuildingType::CityCenter)
		{
			BuildingMesh->SetMaterial(0, buildingRedMat);
			bCanCheck = false;
			OverlappingActors.Empty();
		}
	}
	else
	{
		if (buildingGreenMat)
		{
			if (bBuildingIsAllowedToBeBuilt)
			{
				BuildingMesh->SetMaterial(0, buildingGreenMat);
			}
			bCanCheck = true;
			GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Blue, TEXT("888888888888888888888888"));
		}
	}
}

void ABuilding::CheckCanBuild()
{
	if (valoriaCam)
	{
		if (bCanCheck)
		{
			if (valoriaCam->GetWood() >= wood && valoriaCam->GetStone() >= stone && valoriaCam->GetGold() >= gold && valoriaCam->GetScience() >= science)
			{
				bBuildingIsAllowedToBeBuilt = true;
				GEngine->AddOnScreenDebugMessage(-1, 0.005f, FColor::Green, TEXT("can build"));
			}
			else
			{
				bBuildingIsAllowedToBeBuilt = false;
				GEngine->AddOnScreenDebugMessage(-1, 0.005f, FColor::Green, TEXT("can not build"));
				if (valoriaCam->buildingRef && !bBuildingPlaced)
				{
					valoriaCam->buildingRef->BuildingMesh->SetMaterial(0, buildingRedMat);
				}
			}
		}
		else
		{
			bBuildingIsAllowedToBeBuilt = false;
		}

	}

}

