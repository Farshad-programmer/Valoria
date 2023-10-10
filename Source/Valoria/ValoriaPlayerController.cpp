// Copyright Epic Games, Inc. All Rights Reserved.

#include "ValoriaPlayerController.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "ValoriaCharacter.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"

AValoriaPlayerController::AValoriaPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	//CachedDestination = FVector::ZeroVector;
	//FollowTime = 0.f;
}

void AValoriaPlayerController::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	//if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	//{
	//	Subsystem->AddMappingContext(DefaultMappingContext, 0);
	//}

	bShowMouseCursor = true;
	FInputModeGameOnly inputMode;
	SetInputMode(inputMode);

}

void AValoriaPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();
	// Set up action bindings
	//if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	//{
	//	// Setup mouse input events
	//	EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Started, this, &AValoriaPlayerController::OnInputStarted);
	//	EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Triggered, this, &AValoriaPlayerController::OnSetDestinationTriggered);
	//	EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Completed, this, &AValoriaPlayerController::OnSetDestinationReleased);
	//	EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Canceled, this, &AValoriaPlayerController::OnSetDestinationReleased);
	//	EnhancedInputComponent->BindAction(Deselect, ETriggerEvent::Started, this, &AValoriaPlayerController::OnDeselectStarted);


	//}
}

void AValoriaPlayerController::OnInputStarted()
{
	if (!bIsPlayerSelected) return;
	StopMovement();
}

// Triggered every frame when the input is held down
void AValoriaPlayerController::OnSetDestinationTriggered()
{
	// We flag that the input is being pressed
	FollowTime += GetWorld()->GetDeltaSeconds();
	// We look for the location in the world where the player has pressed the input
	bool bHitSuccessful = false;


	bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);



	// If we hit a surface, cache the location
	if (bHitSuccessful)
	{
		if (bIsPlayerSelected)
		{
			CachedDestination = Hit.Location;
			// Move towards mouse pointer 
			APawn* ControlledPawn = GetPawn();
			if (ControlledPawn != nullptr && !Hit.GetActor()->ActorHasTag("Player"))
			{
				FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
				ControlledPawn->AddMovementInput(WorldDirection, 1.0, false);
			}




		}
		else
		{
			if (Hit.GetActor()->ActorHasTag("Player"))
			{
				bIsPlayerSelected = true;
				/*AValoriaCharacter* valeriaChar = Cast<AValoriaCharacter>(Hit.GetActor());
				if (valeriaChar && valeriaChar->GetMesh())
				{
					valeriaChar->GetMesh()->SetRenderCustomDepth(true);
				}*/
			}
		}

		GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Blue,Hit.GetActor()->GetName());

	}


}

void AValoriaPlayerController::OnSetDestinationReleased()
{
	if (!bIsPlayerSelected) return;
	// If it was a short press
	if (FollowTime <= ShortPressThreshold)
	{
		// We move there and spawn some particles
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);
		if (!Hit.GetActor()->ActorHasTag("Player"))
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
		}
	}

	FollowTime = 0.f;
}

void AValoriaPlayerController::OnDeselectStarted()
{
	/*UGameplayStatics::GetAllActorsOfClass(GetWorld(), AValoriaPlayerController::StaticClass(), characters);
	for (auto character : characters)
	{
		AValoriaPlayerController* valeriaPlayerController = Cast<AValoriaPlayerController>(character);
		if (valeriaPlayerController)
		{
			valeriaPlayerController->bIsPlayerSelected = false;
		}
	}
	AValoriaCharacter* valeriaChar = Cast<AValoriaCharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
	if (valeriaChar)
	{
		valeriaChar->GetMesh()->SetRenderCustomDepth(false);
	}*/
}


//GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Blue,selectedCharacterName);
