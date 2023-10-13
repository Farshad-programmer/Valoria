// Copyright Epic Games, Inc. All Rights Reserved.

#include "ValoriaCharacter.h"
#include "ValoriaCam.h"

#include "AIController.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Navigation/PathFollowingComponent.h"
#include "NiagaraComponent.h"
#include "Buildings/Building.h"
#include "Engine/TargetPoint.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"


AValoriaCharacter::AValoriaCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true); // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	SelectionNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SelectionNiagara"));
	SelectionNiagara->SetupAttachment(RootComponent);
	SelectionNiagara->SetVisibility(false);


	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void AValoriaCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bCanCheckForStartWork)
	{
		CheckCharacterDistanceWithBuilding();
		RotateToBuilding(DeltaSeconds);
	}

}

void AValoriaCharacter::MoveToLocation(const FVector loc, bool canWork, ABuilding* building)
{
	if (building)
	{
		if (building->buildingWorkPointsIndex < building->buildingWorkPoints.Num())
		{
			GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Blue, FString::FromInt(building->buildingWorkPointsIndex));
			locationToWork = building->buildingWorkPoints[building->buildingWorkPointsIndex];
			locationToWork.X += building->GetActorLocation().X;
			locationToWork.Y += building->GetActorLocation().Y;
			locationToWork.Z = 116.f;
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("The building currently has enough workers"));
			return;
		}
	}

	tempLocation = loc;
	if (canWork)
	{
		bCanCheckForStartWork = true;
		buildingRef = building;
	}

	AAIController* DefaultAIController = Cast<AAIController>(GetController());
	if (DefaultAIController)
	{
		if (canWork)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("want work"));
			DefaultAIController->MoveToLocation(locationToWork);
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("location : %s"), *locationToWork.ToString()));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("just walking"));
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, FString::Printf(TEXT("location : %s"), *loc.ToString()));

			DefaultAIController->MoveToLocation(loc);
		}

	}

}


void AValoriaCharacter::CheckCharacterDistanceWithBuilding()
{
	if (buildingRef)
	{
		float distance = buildingRef->GetDistanceTo(this);
		if (distance <= 350.f)
		{
			StartWork();
		}
		else
		{
			workerIssueCounter++;
			if (workerIssueCounter >= 450)
			{
				workerIssueCounter = 0;
				bHasProblemToFindDistanceWithBuilding = true;
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, TEXT("Need to find another point"));
				bIsStartedWork = false;

				AAIController* DefaultAIController = Cast<AAIController>(GetController());
				{
					DefaultAIController->MoveToLocation(tempLocation);
				}

				if (bHasProblemToFindDistanceWithBuilding)
				{
					StartWork();
					bHasProblemToFindDistanceWithBuilding = false;
				}
			}

		}

	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Red, TEXT("CheckCharacterDistanceWithBuilding : building is null"));
	}

}

void AValoriaCharacter::StartWork()
{
	GetCharacterMovement()->StopMovementImmediately();
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("start to work"));
	float distance = buildingRef->GetDistanceTo(this);
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && BuildingAnimation && buildingRef && buildingRef->buildingWorkPointsIndex < buildingRef->buildingWorkPoints.Num() && distance <= 350.f)
	{
		animInstance->Montage_Play(BuildingAnimation, 1.f);
		buildingRef->buildingWorkPointsIndex++;
		buildingRef->workerNumber++;
		buildingRef->buidlingWorkers.Add(this);
		bCanCheckForStartWork = false;
		bIsStartedWork = true;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("worker sticked!!!"));
		bIsStartedWork = false;
	}
	AValoriaCam* valoriaCam = Cast<AValoriaCam>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (valoriaCam)
	{
		valoriaCam->SetIsPlayerSelected(false);
		if (valoriaCam->IsAllNewWorkersStartedWork(valoriaCam->players))
		{
			valoriaCam->DeselectAllCharacters();
		}
	}
}

void AValoriaCharacter::StopWorkAnimation()
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && BuildingAnimation)
	{
		animInstance->Montage_Stop(0.0, BuildingAnimation);
	}
	if (buildingRef && buildingRef->buidlingWorkers.Num() <= 0)
	{
		for (auto Worker : buildingRef->buidlingWorkers)
		{
			UAnimInstance* animIns = Worker->GetMesh()->GetAnimInstance();
			animIns->Montage_Stop(0.0, BuildingAnimation);
		}
	}

}

void AValoriaCharacter::RotateToBuilding(float deltaTime)
{
	if (buildingRef == nullptr)return;

	FRotator actorRotation = GetActorRotation();
	FVector actorLocation = GetActorLocation();
	FRotator FindLookAtRotationOutput = UKismetMathLibrary::FindLookAtRotation(actorLocation, buildingRef->GetActorLocation());
	FRotator RInterpToOutput = FMath::RInterpTo(actorRotation, FindLookAtRotationOutput, deltaTime, 15.f);
	SetActorRotation(FRotator(0.f, FindLookAtRotationOutput.Yaw, 0.f));
}
