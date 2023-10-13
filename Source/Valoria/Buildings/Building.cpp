// Fill out your copyright notice in the Description page of Project Settings.


#include "Building.h"
#include "Kismet/GameplayStatics.h"
#include "Valoria/ValoriaCam.h"

// Sets default values
ABuilding::ABuilding()
{
	PrimaryActorTick.bCanEverTick = true;
	BuildingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BuildingMesh"));
	BuildingMesh->SetupAttachment(RootComponent);
	BuildingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn,ECollisionResponse::ECR_Ignore);
}

void ABuilding::BeginPlay()
{
	Super::BeginPlay();
	valoriaCam = Cast<AValoriaCam>(UGameplayStatics::GetPlayerPawn(this,0));
}

void ABuilding::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsBuildingSpawned)
	{
		bCanPlaceBuilding = true;
		APlayerController* playerController = UGameplayStatics::GetPlayerController(this,0);
		if(playerController)
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
		bCanPlaceBuilding = false;
		if (valoriaCam)
		{
			if (valoriaCam->buildingRef)
			{
				//valoriaCam->buildingRef = nullptr;
				BuildingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn,ECollisionResponse::ECR_Block);
			}
		}
	}
}

