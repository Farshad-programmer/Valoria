// Fill out your copyright notice in the Description page of Project Settings.


#include "ValoriaCam.h"
#include "ValoriaCharacter.h"

#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "NiagaraFunctionLibrary.h"
#include "ValoriaPlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Buildings/Building.h"
#include "Kismet/GameplayStatics.h"
#include "HUD/ValoriaHUD.h"
// Sets default values
AValoriaCam::AValoriaCam()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ValeriaCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));

	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;
}

// Called when the game starts or when spawned
void AValoriaCam::BeginPlay()
{
	Super::BeginPlay();
	//Add Input Mapping Context
	playerController = Cast<APlayerController>(Controller);
	if (playerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(playerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}



}

// Called every frame
void AValoriaCam::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (playerController)
	{
		if (bIsLeftMousePressed)
		{
			AValoriaHUD* valoriaHUD = Cast<AValoriaHUD>(playerController->GetHUD());
			if (valoriaHUD)
			{
				valoriaHUD->MarqueeHeld();
			}
		}
		else
		{

		}

		FHitResult checkCoursorHit;
		bool bHitHapened = playerController->GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, checkCoursorHit);

		if (bHitHapened)
		{
			if (playerController)
			{
				if (checkCoursorHit.GetActor()->ActorHasTag("Building"))
				{
					if (bIsPlayerSelected)
					{
						playerController->CurrentMouseCursor = EMouseCursor::GrabHand;
					}
					else
					{
						playerController->CurrentMouseCursor = EMouseCursor::Default;
					}
				}
				else
				{
					playerController->CurrentMouseCursor = EMouseCursor::Default;
				}
			}

		}

	}

}

void AValoriaCam::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		// Setup mouse input events
		EnhancedInputComponent->BindAction(SelectClickAction, ETriggerEvent::Started, this, &AValoriaCam::OnInputStarted);
		EnhancedInputComponent->BindAction(SelectClickAction, ETriggerEvent::Started, this, &AValoriaCam::OnSelectStarted);
		EnhancedInputComponent->BindAction(SelectClickAction, ETriggerEvent::Completed, this, &AValoriaCam::OnSelectReleased);
		EnhancedInputComponent->BindAction(SelectClickAction, ETriggerEvent::Canceled, this, &AValoriaCam::OnSelectReleased);
		EnhancedInputComponent->BindAction(Deselect, ETriggerEvent::Started, this, &AValoriaCam::OnDeselectStarted);

		EnhancedInputComponent->BindAction(SetDestinationClickAction2, ETriggerEvent::Started, this, &AValoriaCam::OnSetDestinationStarted2);
		EnhancedInputComponent->BindAction(SetDestinationClickAction2, ETriggerEvent::Completed, this, &AValoriaCam::OnSetDestinationReleased2);
	}

}

void AValoriaCam::OnInputStarted()
{

}

void AValoriaCam::DeselectAllCharacters()
{
	if (players.Num() > 0)
	{
		for (auto player : players)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("DeselectAllCharacters "));

			//player->GetMesh()->SetRenderCustomDepth(false);
			player->SetSelectionNiagaraVisibility(false);
			player->SetCheckForStartWork(false);
			bMarqueeSelected = false;
			player->buildingRef = nullptr;
			//bCanMarqueeMove = false;
		}
		players.Empty();
	}
}



void AValoriaCam::OnSelectStarted()
{
	DeselectAllCharacters();
	// We flag that the input is being pressed
	FollowTime += GetWorld()->GetDeltaSeconds();
	// We look for the location in the world where the player has pressed the input
	bCourserHitSuccessful = false;
	bIsLeftMousePressed = true;

	if (playerController)
	{
		bCourserHitSuccessful = playerController->GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);
	}
	// If we hit a surface, cache the location
	if (bCourserHitSuccessful)
	{
		if (bIsPlayerSelected)
		{
			CachedDestination = Hit.Location;
			// Move towards mouse pointer 
			APawn* HitedPawn = Cast<APawn>(Hit.GetActor());
			if (HitedPawn != nullptr && !Hit.GetActor()->ActorHasTag("Player"))
			{
				FVector WorldDirection = (CachedDestination - HitedPawn->GetActorLocation()).GetSafeNormal();
				HitedPawn->AddMovementInput(WorldDirection, 1.0, false);
			}
		}
		if (!bIsPlayerSelected)
		{
			if (Hit.GetActor()->ActorHasTag("Player"))
			{
				bIsPlayerSelected = true;
			}
		}
		if (Hit.GetActor()->ActorHasTag("Player"))
		{
			DeselectAllCharacters();
			if (!bMarqueeSelected)
			{
				AValoriaCharacter* PlayerTemp = Cast<AValoriaCharacter>(Hit.GetActor());
				if (PlayerTemp)
				{
					if (!players.Contains(PlayerTemp))
					{
						bCanMarqueeMove = false;
						players.AddUnique(PlayerTemp);
						players[0]->SetSelectionNiagaraVisibility(true);
						players[0]->SetCheckForStartWork(false);
					}
				}
				if (playerController)
				{
					AValoriaHUD* valoriaHUD = Cast<AValoriaHUD>(playerController->GetHUD());
					if (valoriaHUD)
					{
						valoriaHUD->bCanDrawSelection = false;
						valoriaHUD->ReceiveDrawHUD(0, 0);
					}
				}
			}
		}
		if (Hit.GetActor()->ActorHasTag("Building"))
		{
			ABuilding* building = Cast<ABuilding>(Hit.GetActor());

			if (playerController)
			{
				for (auto player : players)
				{
					player->MoveToLocation(Hit.Location, true, building);
				}
			}
		}
		if (!Hit.GetActor()->ActorHasTag("Building"))
		{
			if (playerController)
			{
				for (auto player : players)
				{
					player->SetCheckForStartWork(false);
					player->buildingRef = nullptr;
				}
			}
		}

		if (!Hit.GetActor()->ActorHasTag("Player"))
		{
			if (playerController)
			{
				AValoriaHUD* valoriaHUD = Cast<AValoriaHUD>(playerController->GetHUD());
				if (valoriaHUD)
				{
					valoriaHUD->bCanDrawSelection = true;
					valoriaHUD->MarqueePressed();
				}
			}
		}
	}
}




void AValoriaCam::OnSelectReleased()
{
	bIsLeftMousePressed = false;
	if (playerController)
	{
		AValoriaHUD* valoriaHUD = Cast<AValoriaHUD>(playerController->GetHUD());
		if (valoriaHUD)
		{
			valoriaHUD->MarqueeReleased();
		}
	}
}

void AValoriaCam::OnDeselectStarted()
{
	DeselectAllCharacters();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Deselect all "));
	bCanMarqueeMove = false;
}

void AValoriaCam::OnSetDestinationStarted2()
{

	// We flag that the input is being pressed
	FollowTime += GetWorld()->GetDeltaSeconds();
	// We look for the location in the world where the player has pressed the input
	bCourserHitSuccessful = false;
	//bIsLeftMousePressed = true;

	if (playerController)
	{
		bCourserHitSuccessful = playerController->GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);
	}
	// If we hit a surface, cache the location
	if (bCourserHitSuccessful)
	{
		if (bIsPlayerSelected)
		{
			CachedDestination = Hit.Location;
			// Move towards mouse pointer 
			APawn* HitedPawn = Cast<APawn>(Hit.GetActor());
			if (HitedPawn != nullptr && !Hit.GetActor()->ActorHasTag("Player"))
			{
				FVector WorldDirection = (CachedDestination - HitedPawn->GetActorLocation()).GetSafeNormal();
				HitedPawn->AddMovementInput(WorldDirection, 1.0, false);
			}
		}
		if (!bIsPlayerSelected)
		{
			if (Hit.GetActor()->ActorHasTag("Player"))
			{
				bIsPlayerSelected = true;
			}
		}
		if (Hit.GetActor()->ActorHasTag("Player"))
		{
			DeselectAllCharacters();
			if (!bMarqueeSelected)
			{
				AValoriaCharacter* PlayerTemp = Cast<AValoriaCharacter>(Hit.GetActor());
				if (PlayerTemp)
				{
					if (!players.Contains(PlayerTemp))
					{
						bCanMarqueeMove = false;
						players.AddUnique(PlayerTemp);
						players[0]->SetSelectionNiagaraVisibility(true);
						players[0]->SetCheckForStartWork(false);
					}


				}
			}
		}
		if (Hit.GetActor()->ActorHasTag("Building"))
		{
			ABuilding* building = Cast<ABuilding>(Hit.GetActor());

			if (playerController)
			{
				for (auto player : players)
				{
					player->MoveToLocation(Hit.Location, true, building);
				}
			}
		}
		if (!Hit.GetActor()->ActorHasTag("Building"))
		{
			if (playerController)
			{
				for (auto player : players)
				{
					player->SetCheckForStartWork(false);
					player->buildingRef = nullptr;
				}
			}
		}
	}
}

void AValoriaCam::OnSetDestinationReleased2()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("OnSetDestinationReleased "));

	bIsLeftMousePressed = false;
	if (playerController)
	{
		AValoriaHUD* valoriaHUD = Cast<AValoriaHUD>(playerController->GetHUD());
		if (valoriaHUD)
		{
			valoriaHUD->MarqueeReleased();
		}
	}


	// We move there and spawn some particles
	if (!bCourserHitSuccessful)return;
	if (!Hit.GetActor()->ActorHasTag("Player"))
	{
		if (players.Num() > 0)
		{
			if (!bMarqueeSelected)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Player Move "));
				players[0]->MoveToLocation(Hit.Location, false, nullptr);
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
			}
			else
			{
				if (bCanMarqueeMove)
				{
					// move Marquee
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Players all Move "));
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, Hit.Location, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
					for (auto player : players)
					{
						player->MoveToLocation(Hit.Location, false, nullptr);
					}
				}

			}

		}

	}
	else if (Hit.GetActor()->ActorHasTag("Building"))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, Hit.Location, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}
}


//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Bool: %s"), bMarqueeSelected ? TEXT("true") : TEXT("false")));

//GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Blue, FString::FromInt(players.Num()));

