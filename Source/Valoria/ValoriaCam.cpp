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
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HUD/ValoriaHUD.h"
#include "Resources/ResourceMaster.h"
#include "Valoria/Buildings/BuildingBanner.h"
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
	/*
	if (buildingRef)
	{
		if (buildingRef->GetBuildingIsAllowedToBeBuilt())
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Green, FString::Printf(TEXT("%s"),*buildingRef->GetName()));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Red, FString::Printf(TEXT("%s"),*buildingRef->GetName()));
		}
	}
	 */




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

		FHitResult checkCoursorHit;
		bool bHitHapened = playerController->GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, checkCoursorHit);

		if (bHitHapened)
		{
			if (playerController)
			{
				if (checkCoursorHit.GetActor()->ActorHasTag("Building") || checkCoursorHit.GetActor()->ActorHasTag("Resource"))
				{
					if (bIsPlayerSelected || bMarqueeSelected)
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

		if (bCanAdjustBuildingBannerPosition && buildingBannerRef)
		{
			FVector bannerLoc = checkCoursorHit.Location;
			if (!buildingBannerRef->bBannerAdjusted)
			{
				bannerLoc.Z += 100.f;
				buildingBannerRef->SetActorLocation(bannerLoc);
			}
			else
			{
				if (buildingBannerRef->buildingRelated)
				{
					//buildingBannerRef->buildingRelated->SetActorLocation(bannerLoc);
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

		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Started, this, &AValoriaCam::OnSetDestinationStarted);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Completed, this, &AValoriaCam::OnSetDestinationReleased);
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

			player->SetSelectionNiagaraVisibility(false);
			player->SetOverlayWidgetVisibility(false);
			bMarqueeSelected = false;
		}
		players.Empty();
	}
	if (!bMovingBanner && bBuildingSelected)
	{

	}


}


bool AValoriaCam::IsAllNewWorkersStartedWork(TArray<AValoriaCharacter*> workers)
{
	if (workers.Num() > 0)
	{
		for (auto worker : workers)
		{
			if (worker->GetIsStartedWork() == false)
			{
				return false;
			}
		}
		return true;
	}
	return false;

}


void AValoriaCam::DestroyAllBanners()
{
	if (AllBanners.Num() > 0)
	{
		for (ABuildingBanner* banner : AllBanners)
		{
			if (banner)
			{
				if (!bAdjustingBanner)
				{
					banner->Destroy();
					adjustingBannerCounter = 0;
				}
			}
		}
	}
}

void AValoriaCam::OnSelectStarted()
{
	DestroyAllBanners();
	if (bAdjustingBanner)
	{
		adjustingBannerCounter++;
		if (adjustingBannerCounter > 0)
		{
			bAdjustingBanner = false;
		}
	}

	if (bRunCustomDepthSpecialMode)
	{
		RenderCustomDepthHandle();
	}

	if (buildingRef)
	{
		if (buildingRef->GetBuildingIsAllowedToBeBuilt())
		{
			buildingRef->SetIsBuildingSpawned(false);
		}

	}

	if (bCanAdjustBuildingBannerPosition)
	{
		bCanAdjustBuildingBannerPosition = false;
		buildingBannerRef->bBannerAdjusted = true;
		buildingBannerRef->buildingRelated->bannerLocation = buildingBannerRef->GetActorLocation();
	}

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
			TArray<AActor*>buildingActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuilding::StaticClass(), buildingActors);
			if (buildingActors.Num() > 0)
			{
				for (auto buildingActor : buildingActors)
				{
					ABuilding* buildingActorCasted = Cast<ABuilding>(buildingActor);
					if (buildingActorCasted)
					{
						if (buildingActorCasted->GetBuildingType() == EBuildingType::Barracks && buildingActorCasted->GetBuildingMesh()->bRenderCustomDepth == true)
						{
							buildingActorCasted->GetBuildingMesh()->SetRenderCustomDepth(false);
						}
					}
				}
			}
			DeselectAllCharacters();
			if (!bMarqueeSelected)
			{
				BP_ConstructionHUD(true, 0);
				AValoriaCharacter* PlayerTemp = Cast<AValoriaCharacter>(Hit.GetActor());
				if (PlayerTemp)
				{
					if (!players.Contains(PlayerTemp))
					{
						bCanMarqueeMove = false;
						players.AddUnique(PlayerTemp);
						players[0]->SetSelectionNiagaraVisibility(true);
						players[0]->SetCheckForStartWork(false);
						players[0]->SetOverlayWidgetVisibility(true);
						if (players[0]->GetIsStartedWork())
						{
							//if (buildingRef == nullptr)return;
							players[0]->StopWorkAnimation();
							if (players[0]->buildingRef && players[0]->GetIsStartedWork() && buildingRef->buildingWorkPointsIndex > 0 && players[0]->GetMesh()->GetAnimInstance()->IsAnyMontagePlaying())
							{
								players[0]->buildingRef->buildingWorkPointsIndex--;
								players[0]->buildingRef->workerNumber--;
								players[0]->buildingRef->buidlingWorkers.Remove(players[0]);
								if (players[0]->buildingRef->buildingWorkPointsIndex < 0)
								{
									players[0]->buildingRef->buildingWorkPointsIndex = 0;
									players[0]->buildingRef->workerNumber = 0;
									players[0]->buildingRef->buidlingWorkers.Empty();
									bIsPlayerSelected = true;
								}
								players[0]->buildingRef = nullptr;
							}

							if (players[0]->resourceRef && players[0]->GetIsStartedWork() && players[0]->resourceRef->buildingWorkPointsIndex > 0 && players[0]->GetMesh()->GetAnimInstance()->IsAnyMontagePlaying())
							{
								players[0]->SetOverlayWidgetVisibility(true);
								players[0]->resourceRef->buildingWorkPointsIndex--;
								players[0]->resourceRef->workerNumber--;
								players[0]->resourceRef->buidlingWorkers.Remove(players[0]);
								if (players[0]->resourceRef->buildingWorkPointsIndex < 0)
								{
									players[0]->resourceRef->buildingWorkPointsIndex = 0;
									players[0]->resourceRef->workerNumber = 0;
									players[0]->resourceRef->buidlingWorkers.Empty();
								}
								players[0]->resourceRef = nullptr;
							}
						}


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

			TArray<AActor*>buildingActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuilding::StaticClass(), buildingActors);
			if (buildingActors.Num() > 0)
			{
				for (auto buildingActor : buildingActors)
				{
					ABuilding* buildingActorCasted = Cast<ABuilding>(buildingActor);
					if (buildingActorCasted)
					{
						if (buildingActorCasted->GetBuildingType() == EBuildingType::Barracks && buildingActorCasted->GetBuildingMesh()->bRenderCustomDepth == true)
						{
							buildingActorCasted->GetBuildingMesh()->SetRenderCustomDepth(false);
						}
					}
				}
			}
			DestroyAllBanners();
			ABuilding* building = Cast<ABuilding>(Hit.GetActor());
			bBuildingSelected = true;
			if (playerController && !bCanPlaceBuilding)
			{
				for (auto player : players)
				{
					player->SetCheckForStartWork(true);
				}
			}
			if (building->GetConstructionIsBuilt())
			{
				if (building->GetBuildingType() == EBuildingType::Barracks)
				{
					GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Yellow, TEXT("Building selected"));
					BP_ConstructionHUD(true, 1);
					building->GetBuildingMesh()->SetRenderCustomDepth(true);
					building->SetIsBuildingSelected(true);
					// spawning Banner
					if (!building->bBuildingHasBanner)
					{
						if (GetWorld() && buildingBannerToSpawn)
						{
							buildingBannerRef = GetWorld()->SpawnActor<ABuildingBanner>(buildingBannerToSpawn, building->GetActorLocation(), building->GetActorRotation());
							if (buildingBannerRef)
							{
								bAdjustingBanner = true;
								bMovingBanner = true;
								building->bBuildingHasBanner = true;
								AllBanners.Add(buildingBannerRef);
								bCanAdjustBuildingBannerPosition = true;
								building->buildingBannerRelated = buildingBannerRef;
								buildingBannerRef->buildingRelated = building;
							}
						}
					}
					else
					{
						if (GetWorld() && buildingBannerToSpawn)
						{
							if (building && building->buildingBannerRelated)
							{
								building->buildingBannerRelated->bBannerAdjusted = true;
								bCanAdjustBuildingBannerPosition = false;
								buildingBannerRef = GetWorld()->SpawnActor<ABuildingBanner>(buildingBannerToSpawn, building->GetActorLocation(), building->GetActorRotation());
								if (buildingBannerRef)
								{
									buildingBannerRef->SetActorLocation(building->bannerLocation);
									AllBanners.Add(buildingBannerRef);
									building->buildingBannerRelated = buildingBannerRef;
									buildingBannerRef->buildingRelated = building;
								}
							}
						}

					}

				}
				else
				{
					DestroyAllBanners();
				}
			}
			else
			{
				BP_ConstructionHUD(false, 0);
			}

		}
		if (!Hit.GetActor()->ActorHasTag("Building"))
		{
			bBuildingSelected = false;
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
					bIsPlayerSelected = false;
				}
			}
		}
		if (!Hit.GetActor()->ActorHasTag("Player") && !Hit.GetActor()->ActorHasTag("Building"))
		{
			TArray<AActor*>buildingActors;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuilding::StaticClass(), buildingActors);
			if (buildingActors.Num() > 0)
			{
				for (auto buildingActor : buildingActors)
				{
					ABuilding* buildingActorCasted = Cast<ABuilding>(buildingActor);
					if (buildingActorCasted)
					{
						if (buildingActorCasted->GetBuildingType() == EBuildingType::Barracks && buildingActorCasted->GetBuildingMesh()->bRenderCustomDepth == true)
						{
							if (!buildingBannerRef->bBannerAdjusted)
							{
								buildingActorCasted->GetBuildingMesh()->SetRenderCustomDepth(false);
							}
							else
							{
								bRunCustomDepthSpecialMode = true;
							}
						}
					}
				}
			}
			BP_ConstructionHUD(false, 0);
		}
	}
}

void AValoriaCam::RenderCustomDepthHandle()
{
	bRunCustomDepthSpecialMode = false;
	TArray<AActor*>buildingActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuilding::StaticClass(), buildingActors);
	if (buildingActors.Num() > 0)
	{
		for (auto buildingActor : buildingActors)
		{
			ABuilding* buildingActorCasted = Cast<ABuilding>(buildingActor);
			if (buildingActorCasted)
			{
				if (buildingActorCasted->GetBuildingType() == EBuildingType::Barracks && buildingActorCasted->GetBuildingMesh()->bRenderCustomDepth == true)
				{
					buildingActorCasted->GetBuildingMesh()->SetRenderCustomDepth(false);
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
	if (buildingRef)
	{
		buildingRef->Destroy();
		bIsPlacingBuidling = false;
		bCanPlaceBuilding = false;
	}
}

void AValoriaCam::OnSetDestinationStarted()
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
		if (Hit.GetActor()->ActorHasTag("Building"))
		{
			ABuilding* building = Cast<ABuilding>(Hit.GetActor());

			if (playerController && !bCanPlaceBuilding && !bIsPlacingBuidling)
			{
				if (players.Num() == 1)
				{
					players[0]->MoveToLocation(Hit.Location, true, building, nullptr);
				}
				else
				{
					return;
				}
			}
		}

		if (Hit.GetActor()->ActorHasTag("Resource"))
		{
			AResourceMaster* resource = Cast<AResourceMaster>(Hit.GetActor());

			if (playerController && !bCanPlaceBuilding && !bIsPlacingBuidling)
			{
				if (players.Num() == 1)
				{
					players[0]->MoveToLocation(Hit.Location, true, nullptr, resource);
				}
				else
				{
					return;
				}
			}
		}
		if (!Hit.GetActor()->ActorHasTag("Building") && !Hit.GetActor()->ActorHasTag("Resource"))
		{
			if (playerController)
			{
				for (auto player : players)
				{
					player->SetCheckForStartWork(false);
					player->buildingRef = nullptr;
					player->resourceRef = nullptr;
				}
			}
		}
	}
}

void AValoriaCam::OnSetDestinationReleased()
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
			if (!bMarqueeSelected && !bCanPlaceBuilding)
			{
				if (!Hit.GetActor()->ActorHasTag("Building") && !Hit.GetActor()->ActorHasTag("Resource"))
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Player Move "));
					players[0]->MoveToLocation(Hit.Location, false, nullptr, nullptr);
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
				}

			}
			else
			{
				if (bCanMarqueeMove)
				{
					if (!Hit.GetActor()->ActorHasTag("Building") && !Hit.GetActor()->ActorHasTag("Resource"))
					{
						// move Marquee
						GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Players all Move "));
						UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, Hit.Location, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
						if (!bCanPlaceBuilding)
						{
							for (auto player : players)
							{
								player->MoveToLocation(Hit.Location, false, nullptr, nullptr);
							}
						}
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



void AValoriaCam::SpawnConstruction(int32 constructionID)
{
	FActorSpawnParameters spawnParameters;
	bIsPlacingBuidling = true;
	switch (constructionID)
	{
	case 0:
		if (houseToSpawn)
		{
			buildingRef = GetWorld()->SpawnActor<ABuilding>(houseToSpawn, GetActorLocation(), FRotator(0.f), spawnParameters);
			if (buildingRef)
			{
				buildingRef->SetIsBuildingSpawned(true);

			}
		}
		break;
	case 1:
		if (BarracksToSpawn)
		{
			buildingRef = GetWorld()->SpawnActor<ABuilding>(BarracksToSpawn, GetActorLocation(), FRotator(0.f), spawnParameters);
			if (buildingRef)
			{
				buildingRef->SetIsBuildingSpawned(true);
			}
		}
		break;

	default:
		break;
	}
}

void AValoriaCam::UpdateWood(bool plus, int32 amount)
{
	if (plus)
	{
		wood += amount;
	}
	else
	{
		wood -= amount;
	}
}

void AValoriaCam::UpdateGold(bool plus, int32 amount)
{
	if (plus)
	{
		gold += amount;
	}
	else
	{
		gold -= amount;
	}
}

void AValoriaCam::UpdateStone(bool plus, int32 amount)
{
	if (plus)
	{
		stone += amount;
	}
	else
	{
		stone -= amount;
	}
}

void AValoriaCam::UpdateScience(bool plus, int32 amount)
{
	if (plus)
	{
		science += amount;
	}
	else
	{
		science -= amount;
	}
}


//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Bool: %s"), bMarqueeSelected ? TEXT("true") : TEXT("false")));

//GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Blue, FString::FromInt(players.Num()));

