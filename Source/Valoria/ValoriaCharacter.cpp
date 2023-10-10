// Copyright Epic Games, Inc. All Rights Reserved.

#include "ValoriaCharacter.h"

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
	    CheckCharacterDistanceWithBuilding(buildingRef);
		RotateToBuilding(DeltaSeconds);
    }
	
}

void AValoriaCharacter::MoveToLocation(const FVector loc,bool canWork,ABuilding* building)
{
	AAIController* DefaultAIController = Cast<AAIController>(GetController());

	if (DefaultAIController)
	{
		DefaultAIController->MoveToLocation(loc);
	}
	if (canWork)
	{
		bCanCheckForStartWork = true;
		buildingRef = building;
	}
}

void AValoriaCharacter::CheckCharacterDistanceWithBuilding(ABuilding* building)
{
	if (building)
	{
		float distance = building->GetDistanceTo(this);
		if (distance <= 300.f)
		{
			GetCharacterMovement()->StopMovementImmediately();
			GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("start to work"));
			bCanCheckForStartWork = false;
			//TODo
		}
	}
	
}

void AValoriaCharacter::RotateToBuilding(float deltaTime)
{
	if (buildingRef == nullptr)return;
	
	FRotator actorRotation = GetActorRotation();
	FVector actorLocation = GetActorLocation();
	FRotator FindLookAtRotationOutput = UKismetMathLibrary::FindLookAtRotation(actorLocation,buildingRef->GetActorLocation());
	FRotator RInterpToOutput = FMath::RInterpTo(actorRotation, FindLookAtRotationOutput, deltaTime, 15.f);
	//SetActorRotation(FRotator(actorRotation.Roll,RInterpToOutput.Yaw,actorRotation.Pitch));
	SetActorRotation(FRotator(0.f,FindLookAtRotationOutput.Yaw,0.f));
}
