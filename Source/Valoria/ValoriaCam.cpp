// Fill out your copyright notice in the Description page of Project Settings.


#include "ValoriaCam.h"
#include "ValoriaCharacter.h"

#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "ValoriaPlayerController.h"
#include "Buildings/Building.h"
#include "Characters/ValoriaInfantry.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HUD/ValoriaHUD.h"
#include "Resources/ResourceMaster.h"
#include "Valoria/Buildings/BuildingBanner.h"


AValoriaCam::AValoriaCam()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	ValeriaCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));

	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;
}
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
		UpdateBannerLocationPlacement(checkCoursorHit);

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
void AValoriaCam::UpdateBannerLocationPlacement(FHitResult checkCoursorHit)
{
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
void AValoriaCam::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		// Setup mouse input events
		EnhancedInputComponent->BindAction(SelectClickAction, ETriggerEvent::Started, this, &AValoriaCam::OnSelectStarted);
		EnhancedInputComponent->BindAction(SelectClickAction, ETriggerEvent::Completed, this, &AValoriaCam::OnSelectReleased);
		EnhancedInputComponent->BindAction(SelectClickAction, ETriggerEvent::Canceled, this, &AValoriaCam::OnSelectReleased);
		EnhancedInputComponent->BindAction(Deselect, ETriggerEvent::Started, this, &AValoriaCam::OnDeselectStarted);

		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Started, this, &AValoriaCam::OnSetDestinationStarted);
		EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Completed, this, &AValoriaCam::OnSetDestinationReleased);
	}

}
void AValoriaCam::DeselectAllCharacters()
{
	if (players.Num() > 0)
	{
		for (auto player : players)
		{
			if (player)
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("DeselectAllCharacters "));
				player->SetSelectionNiagaraVisibility(false);
				player->SetOverlayWidgetVisibility(false);
				player->SetIsSelected(false);
				bMarqueeSelected = false;
			}
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
			UStaticMeshComponent* buildingMesh = buildingActorCasted->GetBuildingMesh();
			if (buildingMesh && buildingMesh->bRenderCustomDepth)
			{
				if (buildingActorCasted->GetBuildingType() == EBuildingType::Barracks)
				{
					if (buildingBannerRef && !buildingBannerRef->bBannerAdjusted)
					{
						buildingMesh->SetRenderCustomDepth(false);
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
}
void AValoriaCam::HandleBannerHit()
{
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
}
void AValoriaCam::SpawnBanner(ABuilding* building)
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
void AValoriaCam::UpdateBannerPosition(ABuilding* building)
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
void AValoriaCam::OnSelectStarted()
{
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
		HandleHittedActors();
	}
}
void AValoriaCam::HandleHittedActors()
{
	HandleBannerHit();
	HandlePlayerHit();
	HandleBuildingHit();
	HandleNonBuildingHit();
	HandleNonPlayerHit();
	HandleNonPlayerAndBuildingHit();
}
void AValoriaCam::HandleBuildingHit()
{
	if (Hit.GetActor() != nullptr && Hit.GetActor()->ActorHasTag("Building"))
	{
		DeselectAllBuildings();
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
				HandleBarracksHit(building);
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
}
void AValoriaCam::HandleNonBuildingHit()
{
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
}
void AValoriaCam::HandleNonPlayerHit()
{
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
}
void AValoriaCam::HandleNonPlayerAndBuildingHit()
{
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
void AValoriaCam::HandlePlayerHit()
{
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
}
void AValoriaCam::HandleBarracksHit(ABuilding* building)
{
	GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Yellow, TEXT("Building selected"));
	building->BP_ConstructionHUD(true, 1, buildingRef);
	building->GetBuildingMesh()->SetRenderCustomDepth(true);
	building->SetIsBuildingSelected(true);
	// spawning Banner
	if (!building->bBuildingHasBanner)
	{
		SpawnBanner(building);
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
				UpdateBannerPosition(building);
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
	MakeMarqueeReleased();
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
	DefineLocationToMove();
}
void AValoriaCam::DefineLocationToMove()
{
	// We flag that the input is being pressed
	FollowTime += GetWorld()->GetDeltaSeconds();
	// We look for the location in the world where the player has pressed the input
	bCourserHitSuccessful = GetCursorHitResult();

	// If we hit a surface, cache the location
	if (bCourserHitSuccessful)
	{
		if (bIsPlayerSelected)
		{
			MoveSelectedPawnTowardsCursor();
		}
		else
		{
			SelectPlayerIfHitActorIsPlayer();
		}

		if (Hit.GetActor() != nullptr)
		{

			if (Hit.GetActor()->ActorHasTag("Building"))
			{
				MovePlayersToBuildingLocation();
			}
			else if (Hit.GetActor()->ActorHasTag("Resource"))
			{
				MovePlayersToResourceLocation();
			}

			else if (Hit.GetActor()->ActorHasTag("AIBase"))
			{
				MovePlayersToAIBaseLocation();
			}

			else if (Hit.GetActor()->ActorHasTag("AI"))
			{
				MovePlayersToAILocation();
			}
		}
	}
}
void AValoriaCam::SelectPlayerIfHitActorIsPlayer()
{
	if (!bIsPlayerSelected)
	{
		if (Hit.GetActor() != nullptr && Hit.GetActor()->ActorHasTag("Player"))
		{
			bIsPlayerSelected = true;
		}
	}
}
void AValoriaCam::MovePlayersToBuildingLocation()
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
void AValoriaCam::MovePlayersToResourceLocation()
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
void AValoriaCam::MovePlayersToAIBaseLocation()
{
	ABuilding* building = Cast<ABuilding>(Hit.GetActor());

	if (playerController)
	{
		for (auto player : players)
		{
			player->MoveToLocation(Hit.Location, false, building, nullptr, false, nullptr, true);
		}
	}
}
void AValoriaCam::MovePlayersToAILocation()
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
void AValoriaCam::MakeMarqueeReleased()
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
void AValoriaCam::OnSetDestinationReleased()
{
	if (Hit.GetActor() != nullptr && Hit.GetActor()->ActorHasTag("AIBase")) return;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("OnSetDestinationReleased "));
	MakeMarqueeReleased();

	// We move there and spawn some particles
	if (!bCourserHitSuccessful)return;
	if (Hit.GetActor() != nullptr && !Hit.GetActor()->ActorHasTag("Player") && !Hit.GetActor()->ActorHasTag("AI"))
	{
		MovePlayerOnMap();
	}
	else if (Hit.GetActor() != nullptr && Hit.GetActor()->ActorHasTag("Building"))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, Hit.Location, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}
}
void AValoriaCam::MovePlayerOnMap()
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
				players[0]->SetCanRotateToBuilding(false);
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
								player->SetCanRotateToBuilding(false);
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
bool AValoriaCam::GetCursorHitResult()
{
	if (playerController)
	{
		return playerController->GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);
	}
	return false;
}
void AValoriaCam::MoveSelectedPawnTowardsCursor()
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

void AValoriaCam::HandleNeighborRequest(AValoriaAI* AIRef, int32 woodRequestNumber, int32 goldRequestNumber,
	int32 stoneRequestNumber, int32 ScienceRequestNumber)
{
	bHasNewReqeustFromNeighbor = true;
	woodRequest = woodRequestNumber;
	stoneRequest = stoneRequestNumber;
	goldRequest = goldRequestNumber;
	scienceRequest = ScienceRequestNumber;

	// todo
}






//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("Bool: %s"), bMarqueeSelected ? TEXT("true") : TEXT("false")));
//GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Blue, FString::FromInt(players.Num()));
//FString MyFString = capitalName;
//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, MyFString);

