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
#include "Characters/ValoriaInfantry.h"
#include "Components/BoxComponent.h"
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
		UpdateMouseCursor(checkCoursorHit);

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

				}
			}

		}



	}
}

void AValoriaCam::UpdateMouseCursor(FHitResult& checkCoursorHit)
{
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
			else if (checkCoursorHit.GetActor()->ActorHasTag("AI") || checkCoursorHit.GetActor()->ActorHasTag("AIBase"))
			{
				if (bIsPlayerSelected || bMarqueeSelected)
				{
					playerController->CurrentMouseCursor = EMouseCursor::Hand;
				}
				else
				{
					playerController->CurrentMouseCursor = EMouseCursor::Default;
				}
			}
			else
			{
				playerController->CurrentMouseCursor = EMouseCursor::Default;
				if (checkCoursorHit.GetActor()->ActorHasTag("Banner"))
				{
					bMouseIsOnBanner = true;
				}
				else
				{
					bMouseIsOnBanner = false;
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
			player->SetIsSelected(false);
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
	if (!bMouseIsOnBanner && !bMovingBanner)
	{
		for (ABuildingBanner* banner : AllBanners)
		{
			if (banner)
			{
				if (!bAdjustingBanner)
				{
					banner->Destroy();
				}
			}
		}
		AllBanners.Empty();
	}
}

void AValoriaCam::DeselectAllBuildings()
{
	TArray<AActor*>buildingActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuilding::StaticClass(), buildingActors);

	for (auto buildingActor : buildingActors)
	{
		ABuilding* buildingActorCasted = Cast<ABuilding>(buildingActor);

		if (buildingActorCasted && buildingActorCasted->GetBuildingMesh())
		{
			if (buildingActorCasted->GetBuildingType() == EBuildingType::Barracks && buildingActorCasted->GetBuildingMesh()->bRenderCustomDepth)
			{
				if (buildingBannerRef && !buildingBannerRef->bBannerAdjusted)
				{
					buildingActorCasted->GetBuildingMesh()->SetRenderCustomDepth(false);
					buildingActorCasted->BP_ConstructionHUD(false, 0, nullptr);
				}
				else
				{
					bRunCustomDepthSpecialMode = true;
				}
			}
		}
	}
}

void AValoriaCam::OnSelectStarted()
{

	/*if (Hit.GetActor())
	{
		if (!Hit.GetActor()->ActorHasTag("Banner") && !Hit.GetActor()->ActorHasTag("Building") && !Hit.GetActor()->ActorHasTag("Player"))
		{
			DestroyAllBanners();
		}
	}*/

	DestroyAllBanners();


	if (bAdjustingBanner && !bMouseIsOnBanner)
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

	if (bCanAdjustBuildingBannerPosition && !bMouseIsOnBanner)
	{
		bCanAdjustBuildingBannerPosition = false;
		buildingBannerRef->bBannerAdjusted = true;
		buildingBannerRef->buildingRelated->bannerLocation = buildingBannerRef->GetActorLocation();
		bMovingBanner = true;
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
			if (Hit.GetActor() != nullptr && Hit.GetActor()->ActorHasTag("Player"))
			{
				bIsPlayerSelected = true;
				bMovingBanner = false;
			}
		}


		if (Hit.GetActor() != nullptr && Hit.GetActor()->ActorHasTag("Banner"))
		{
			buildingBannerRef = Cast<ABuildingBanner>(Hit.GetActor());
			if (buildingBannerRef)
			{
				GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Yellow, TEXT("Banner"));
				buildingBannerRef->bBannerAdjusted = false;
				bCanAdjustBuildingBannerPosition = true;
				adjustingBannerCounter = 0;
				bMovingBanner = true;
			}
		}

		if (Hit.GetActor() != nullptr && Hit.GetActor()->ActorHasTag("Player"))
		{
			bMovingBanner = false;
			DestroyAllBanners();
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
						buildingActorCasted->BP_ConstructionHUD(false, 0, nullptr);
					}
				}
			}
			DeselectAllCharacters();
			if (!bMarqueeSelected)
			{
				if (Hit.GetActor() != nullptr && Hit.GetActor()->ActorHasTag("Worker"))
				{
					BP_ConstructionHUD(true, 0, nullptr);
				}
				PlayerTemp = Cast<AValoriaCharacter>(Hit.GetActor());
				if (PlayerTemp)
				{
					if (!players.Contains(PlayerTemp))
					{
						bCanMarqueeMove = false;
						players.AddUnique(PlayerTemp);
						players[0]->SetSelectionNiagaraVisibility(true);
						players[0]->SetCheckForStartWork(false);
						players[0]->SetOverlayWidgetVisibility(true);
						players[0]->SetCanRotateToEnemy(false);
						players[0]->SetCanCheckDistanceWithAI(false);
						players[0]->SetIsSelected(true);

						if (players[0]->GetIsStartedWork())
						{
							players[0]->StopWorkAnimation();
							if (players[0]->buildingRef && players[0]->GetIsStartedWork() && buildingRef && players[0]->GetMesh()->GetAnimInstance()->IsAnyMontagePlaying())
							{
								players[0]->buildingRef->buildingWorkPointsIndex--;
								players[0]->buildingRef->workerNumber--;
								GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Orange, FString::FromInt(players[0]->buildingRef->workerNumber));
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
		if (Hit.GetActor() != nullptr && Hit.GetActor()->ActorHasTag("Building"))
		{
			bMovingBanner = false;
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
						buildingActorCasted->BP_ConstructionHUD(false, 0, nullptr);
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
					building->BP_ConstructionHUD(true, 1, buildingRef);
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
								GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Orange, TEXT("New Banner spawned!"));
								FVector bannerLoc = building->flagStarterPoint->GetComponentLocation();
								bannerLoc.Z += 100.f;
								building->bBuildingHasBanner = true;
								AllBanners.Add(buildingBannerRef);
								building->buildingBannerRelated = buildingBannerRef;
								buildingBannerRef->buildingRelated = building;
								buildingBannerRef->SetActorLocation(bannerLoc);
								building->bannerLocation = bannerLoc;
							}
						}
					}
					else
					{
						GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Green, TEXT("building currently has an Banner"));
						if (GetWorld() && buildingBannerToSpawn)
						{
							if (!building)
							{
								GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, TEXT("Error ! building NULL"));
								return;
							}
							if (!building->buildingBannerRelated)
							{
								building->buildingBannerRelated = buildingBannerRef;
							}

							if (building && building->buildingBannerRelated)
							{
								building->buildingBannerRelated->bBannerAdjusted = true;
								bCanAdjustBuildingBannerPosition = false;
								FActorSpawnParameters SpawnParameters;
								SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
								buildingBannerRef = GetWorld()->SpawnActor<ABuildingBanner>(buildingBannerToSpawn, building->GetActorLocation(), building->GetActorRotation(), SpawnParameters);
								if (buildingBannerRef)
								{
									GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Green, TEXT("Banner Matched"));
									building->buildingBannerRelated = buildingBannerRef;
									buildingBannerRef->SetActorLocation(building->bannerLocation);
									AllBanners.Add(buildingBannerRef);
									buildingBannerRef->buildingRelated = building;
								}
								else
								{
									GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red, TEXT("Banner Error Happened"));
								}
							}
							else
							{
								GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red, TEXT(" ERROR building->buildingBannerRelated"));
							}
						}
						else
						{
							GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Red, TEXT("buildingBannerToSpawn null ERROR"));
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
				BP_ConstructionHUD(false, 0, nullptr);
			}

		}
		if (Hit.GetActor() != nullptr && !Hit.GetActor()->ActorHasTag("Building"))
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
			DeselectAllBuildings();
		}

		if (Hit.GetActor() != nullptr && !Hit.GetActor()->ActorHasTag("Player"))
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
		if (Hit.GetActor() != nullptr && !Hit.GetActor()->ActorHasTag("Player") && !Hit.GetActor()->ActorHasTag("Building"))
		{
			if (!Hit.GetActor()->ActorHasTag("Banner"))
			{
				bMovingBanner = false;
			}
			DeselectAllBuildings();

			BP_ConstructionHUD(false, 0, nullptr);
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
	if (buildingRef && buildingRef->GetIsBuildingSpawned())
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
			if (Hit.GetActor() != nullptr && Hit.GetActor()->ActorHasTag("Player"))
			{
				bIsPlayerSelected = true;
			}
		}
		if (Hit.GetActor() != nullptr && Hit.GetActor()->ActorHasTag("Building"))
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
		if (Hit.GetActor() != nullptr && Hit.GetActor()->ActorHasTag("Resource"))
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

		if (Hit.GetActor() != nullptr && Hit.GetActor()->ActorHasTag("AIBase"))
		{
			ABuilding* building = Cast<ABuilding>(Hit.GetActor());

			if (playerController)
			{
				if (players.Num() == 1)
				{
					players[0]->MoveToLocation(Hit.Location, false, building, nullptr,false,nullptr,true);
				}
				else
				{
					return;
				}
			}
		}

		if (Hit.GetActor() != nullptr && Hit.GetActor()->ActorHasTag("AI"))
		{

			if (playerController && !bCanPlaceBuilding && !bIsPlacingBuidling)
			{
				for (auto player : players)
				{
					if (player)
					{
						player->SetRunAway(false);
						player->MoveToLocation(Hit.GetActor()->GetActorLocation(), false, nullptr, nullptr, true, Hit.GetActor());
					}
				}
			}
		}

		// I need to check !Hit.GetActor()->ActorHasTag("AI") it still need more test to see if I need to keep it or no !
		/*if (Hit.GetActor() != nullptr && !Hit.GetActor()->ActorHasTag("Building") && !Hit.GetActor()->ActorHasTag("Resource") && !Hit.GetActor()->ActorHasTag("AI"))
		{
			if (playerController)
			{
				for (auto player : players)
				{
					player->SetRunAway(true);
					player->SetCheckForStartWork(false);
					player->buildingRef = nullptr;
					player->resourceRef = nullptr;
				}
			}
		}*/
	}
}

void AValoriaCam::OnSetDestinationReleased()
{
	if(Hit.GetActor() != nullptr && Hit.GetActor()->ActorHasTag("AIBase")) return;


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
	if (Hit.GetActor() != nullptr && !Hit.GetActor()->ActorHasTag("Player") && !Hit.GetActor()->ActorHasTag("AI"))
	{
		if (players.Num() > 0)
		{
			if (!bMarqueeSelected && !bCanPlaceBuilding)
			{
				if (!Hit.GetActor()->ActorHasTag("Building") && !Hit.GetActor()->ActorHasTag("Resource"))
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Player Move "));
					players[0]->MoveToLocation(Hit.Location, false, nullptr, nullptr);
					players[0]->SetRunAway(true);
					players[0]->SetCanRotateToEnemy(false);
					players[0]->SetCanCheckDistanceWithAI(false);
					UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
				}

			}
			else
			{
				if (bCanMarqueeMove)
				{
					// I need to check !Hit.GetActor()->ActorHasTag("AI") it still need more test to see if I need to keep it or no !
					if (Hit.GetActor() != nullptr && !Hit.GetActor()->ActorHasTag("Building") && !Hit.GetActor()->ActorHasTag("Resource") && !Hit.GetActor()->ActorHasTag("AI"))
					{
						// move Marquee
						GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Players all Move "));
						UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, Hit.Location, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
						if (!bCanPlaceBuilding)
						{
							for (auto player : players)
							{
								if (player && Hit.bBlockingHit)
								{
									player->SetRunAway(true);
									player->SetCanRotateToEnemy(false);
									player->SetCanCheckDistanceWithAI(false);
									player->MoveToLocation(Hit.Location, false, nullptr, nullptr);
								}
							}
						}
					}


				}

			}

		}

	}
	else if (Hit.GetActor() != nullptr && Hit.GetActor()->ActorHasTag("Building"))
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
		if (CityCenterToSpawn)
		{
			buildingRef = GetWorld()->SpawnActor<ABuilding>(CityCenterToSpawn, GetActorLocation(), FRotator(0.f), spawnParameters);
			if (buildingRef)
			{
				buildingRef->SetIsBuildingSpawned(true);
			}
		}
		break;
	case 1:
		if (houseToSpawn)
		{
			buildingRef = GetWorld()->SpawnActor<ABuilding>(houseToSpawn, GetActorLocation(), FRotator(0.f), spawnParameters);
			if (buildingRef)
			{
				buildingRef->SetIsBuildingSpawned(true);
			}
		}
		break;
	case 2:
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

void AValoriaCam::SpawnSoldier(int32 soldierCode, ABuilding* building)
{
	if (building == nullptr)return;
	switch (soldierCode)
	{
	case 1:

		break;
	case 2:
		if (valoriaSwordSoldierToSpawn && GetWorld())
		{
			FActorSpawnParameters spawnParameters;
			//spawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			AValoriaInfantry* valoriaInfantry = GetWorld()->SpawnActor<AValoriaInfantry>(valoriaSwordSoldierToSpawn, building->characterStarterPoint->GetComponentLocation(), building->GetActorRotation());
			if (valoriaInfantry)
			{
				valoriaInfantry->MoveToLocation(building->bannerLocation, false, nullptr, nullptr);
			}
		}
		break;
	case 3:
		if (valoriaSpearmanToSpawn && GetWorld())
		{
			FActorSpawnParameters spawnParameters;
			//spawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			AValoriaInfantry* valoriaInfantry = GetWorld()->SpawnActor<AValoriaInfantry>(valoriaSpearmanToSpawn, building->characterStarterPoint->GetComponentLocation(), building->GetActorRotation());
			if (valoriaInfantry)
			{
				valoriaInfantry->MoveToLocation(building->bannerLocation, false, nullptr, nullptr);
			}
		}
		break;
	case 4:

		break;
	case 5:
		if (valoriaCommanderToSpawn && GetWorld())
		{
			FActorSpawnParameters spawnParameters;
			//spawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			AValoriaInfantry* valoriaInfantry = GetWorld()->SpawnActor<AValoriaInfantry>(valoriaCommanderToSpawn, building->characterStarterPoint->GetComponentLocation(), building->GetActorRotation());
			if (valoriaInfantry)
			{
				valoriaInfantry->MoveToLocation(building->bannerLocation, false, nullptr, nullptr);
			}
		}
		break;
	default:
		break;
	}
}




//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Bool: %s"), bMarqueeSelected ? TEXT("true") : TEXT("false")));

//GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Blue, FString::FromInt(players.Num()));




//FString MyFString = capitalName;
//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, MyFString);

