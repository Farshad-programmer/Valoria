// Fill out your copyright notice in the Description page of Project Settings.


#include "Building.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Valoria/ValoriaCam.h"
#include "Valoria/ValoriaCharacter.h"
#include "Valoria/MapBorder/MapBorder.h"
#include "Kismet/KismetMathLibrary.h"
#include "Valoria/AI/AValoriaAI.h"


// BP_House az aval bayad besazamesh , yani delete va recreate beshe

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
	flagStarterPoint = CreateDefaultSubobject<USceneComponent>(TEXT("flag Starter Point"));
	flagStarterPoint->SetupAttachment(BuildingMesh);
	characterStarterPoint = CreateDefaultSubobject<USceneComponent>(TEXT("character Starter Point"));
	characterStarterPoint->SetupAttachment(BuildingMesh);
	if (flagStarterPoint)
	{
		flagStarterPoint->SetRelativeLocation(FVector(-310.f, 0.f, 0.f));

	}
	BuildingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
}
void ABuilding::DamageBuilding(float damage)
{
	constructionCounter -= damage;
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
	if (bConstructionProgressStarted)
	{
		ConstructionProcess(DeltaTime);
	}
	else
	{
		if (bIsBuildingSpawned)
		{
			MovingBuildingToFindRightPlace();
		}
		else
		{
			PlacingBuilding();
		}
	}
}
void ABuilding::ConstructionProcess(float DeltaTime)
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
		if (valoriaAIRef && buildingOwner != EBuildingOwner::self)
		{
			valoriaAIRef->SetHasBarracks(true);
			valoriaAIRef->barracksLocation.Add(this->GetActorLocation() + FVector(500.f, 500.f, 500.f));
		}
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
void ABuilding::MovingBuildingToFindRightPlace()
{
	LineTraceFloorCheckers();
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
void ABuilding::UpdateBuildingNeeds()
{
	if (bUpdatingNeeds)
	{
		bUpdatingNeeds = false;
		valoriaCam->UpdateGold(false, gold);
		valoriaCam->UpdateScience(false, science);
		valoriaCam->UpdateStone(false, stone);
		valoriaCam->UpdateWood(false, wood);
	}
}
void ABuilding::PlacingBuilding()
{
	if (valoriaCam && valoriaCam->buildingRef)
	{
		if (bBuildingIsAllowedToBeBuilt)
		{
			bBuildingIsAllowedToBeBuilt = false;
			if (valoriaCam->buildingRef)
			{
				valoriaCam->SetIsPlacingBuidling(false);
				BuildingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
				if (level1Mesh && !bConstructionIsBuilt && constructionCounter <= 0.f)
				{
					BuildingMesh->SetStaticMesh(level1Mesh);
					UpdateBuildingNeeds();
					if (buildingMat)
					{
						BuildingMesh->SetMaterial(0, buildingMat);
					}
					if (BorderRef)
					{
						BorderRef->borderStatus = EBorderStatus::self;
						BorderRef->bBorderHasCityCenter = true;
					}
					if (valoriaCam->PlayerTemp)
					{
						valoriaCam->PlayerTemp->MoveToLocation(this->GetActorLocation(), true, this, nullptr);
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
void ABuilding::LineTraceFloorCheckers()
{
	if (GetWorld())
	{
		FHitResult hitResult1;
		FVector start1 = GetActorLocation() + FVector(0.f, -buildingRadius, 0.f);
		FVector end1 = start1 + FVector(0.f, 0.f, -buildingHeight);
		FCollisionQueryParams params1;
		params1.AddIgnoredActor(this);
		GetWorld()->LineTraceSingleByChannel(hitResult1, start1, end1, ECollisionChannel::ECC_WorldStatic, params1);
		//DrawDebugLine(GetWorld(), start1, end1, FColor::Yellow, true, -1, 0, 20.f);
		if (hitResult1.bBlockingHit)
		{
			bEdge1 = true;
		}
		else
		{
			bEdge1 = false;
		}
		FHitResult hitResult2;
		FVector start2 = GetActorLocation() + FVector(0.f, buildingRadius, 0.f);
		FVector end2 = start2 + FVector(0.f, 0.f, -buildingHeight);
		FCollisionQueryParams params2;
		params2.AddIgnoredActor(this);
		GetWorld()->LineTraceSingleByChannel(hitResult2, start2, end2, ECollisionChannel::ECC_WorldStatic, params2);
		//DrawDebugLine(GetWorld(),start2, end2, FColor::Yellow, true, -1, 0, 20.f);
		if (hitResult1.bBlockingHit)
		{
			bEdge2 = true;
		}
		else
		{
			bEdge2 = false;
		}

		FHitResult hitResult3;
		FVector start3 = GetActorLocation() + FVector(-buildingRadius, 0.f, 0.f);
		FVector end3 = start3 + FVector(0.f, 0.f, -buildingHeight);
		FCollisionQueryParams params3;
		params3.AddIgnoredActor(this);
		GetWorld()->LineTraceSingleByChannel(hitResult3,start3, end3, ECollisionChannel::ECC_WorldStatic, params3);
		//DrawDebugLine(GetWorld(), start3, end3, FColor::Yellow, true, -1, 0, 20.f);
		if (hitResult3.bBlockingHit)
		{
			bEdge3 = true;
		}
		else
		{
			bEdge3 = false;
		}

		FHitResult hitResult4;
		FVector start4 = GetActorLocation() + FVector(buildingRadius, 0.f, 0.f);
		FVector end4 = start4 + FVector(0.f, 0.f, -buildingHeight);
		FCollisionQueryParams params4;
		params4.AddIgnoredActor(this);
		GetWorld()->LineTraceSingleByChannel(hitResult4, start4, end4, ECollisionChannel::ECC_WorldStatic, params4);
		//DrawDebugLine(GetWorld(), start4, end4, FColor::Yellow, true, -1, 0, 20.f);
		if (hitResult4.bBlockingHit)
		{
			bEdge4 = true;
		}
		else
		{
			bEdge4 = false;
		}
	}
	if (bEdge1 && bEdge2 && bEdge3 && bEdge4)
	{
		bBuildingIsAllowedToBeBuilt = true;
		if (valoriaCam->buildingRef && !bBuildingPlaced && buildingGreenMat)
		{
			valoriaCam->buildingRef->BuildingMesh->SetMaterial(0, buildingGreenMat);
		}
	}
	else
	{
		bBuildingIsAllowedToBeBuilt = false;
		if (valoriaCam->buildingRef && !bBuildingPlaced && buildingRedMat)
		{
			valoriaCam->buildingRef->BuildingMesh->SetMaterial(0, buildingRedMat);
		}
	}
}


