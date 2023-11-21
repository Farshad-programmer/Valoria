// Copyright Epic Games, Inc. All Rights Reserved.

#include "ValoriaCharacter.h"
#include "ValoriaCam.h"

#include "AIController.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NiagaraComponent.h"
#include "Buildings/Building.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"
#include "AI/AValoriaAI.h"
#include "Valoria/Resources/ResourceMaster.h"
#include "Components/WidgetComponent.h"
#include "Animation/AnimMontage.h"
#include "Buildings/CityCenter.h"
#include "Components/BoxComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"



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

	Weapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon"));
	Weapon->SetupAttachment(GetMesh(), "RightHandWeaponSocket");
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Collider"));
	WeaponCollider->SetupAttachment(GetMesh(), "RightHandWeaponSocket");
	WeaponCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);


	SelectionNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SelectionNiagara"));
	SelectionNiagara->SetupAttachment(RootComponent);
	SelectionNiagara->SetVisibility(false);

	Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Overlay Widget"));
	Widget->SetupAttachment(GetMesh());
	Widget->SetVisibility(false);

	enemyDetector = CreateDefaultSubobject<USphereComponent>(TEXT("Enemy Detector"));
	enemyDetector->SetupAttachment(RootComponent);
	enemyDetector->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	enemyDetector->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	enemyDetector->SetSphereRadius(5.f);

	pawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("Pawn Sensing"));
	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}




void AValoriaCharacter::BeginPlay()
{
	Super::BeginPlay();

	enemyStatus = EAIStatus::neutral;
	health = maxHealth;
	WeaponCollider->OnComponentBeginOverlap.AddDynamic(this, &AValoriaCharacter::WeaponBeginOverlap);
	enemyDetector->OnComponentBeginOverlap.AddDynamic(this, &AValoriaCharacter::EnemyDetectorBeginOverlap);

	if (ActorHasTag("Player"))
	{
		capitalCode = 1;
	}

	pawnSensing->OnSeePawn.AddDynamic(this, &AValoriaCharacter::OnSeePawn);

	FTimerHandle checkNearEnemiesHandle;
	GetWorldTimerManager().SetTimer(checkNearEnemiesHandle, this, &AValoriaCharacter::CheckAllNearEnemies, 3.f, true);


}

void AValoriaCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bCanCheckForStartWork)
	{
		RotateToBuilding(DeltaSeconds);
		RotateToResource(DeltaSeconds);
	}
	if (bCanCheckDistanceWithAI)
	{
		CheckCharacterDistanceWithAI();
	}
	if (bCanRotateToEnemy)
	{
		RotateToEnemy(DeltaSeconds);
		//GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Red, TEXT("RotateToEnemy "));
	}
	if (bCanCheckForStartDestroy)
	{
		if (!bCanRotateToEnemy)
		{
			if (!bFacingUnitEnemy)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Red, TEXT("RotateToBuilding "));
				CheckCharacterDistanceWithBuildingToDestroy();
				RotateToBuilding(DeltaSeconds);
			}
			else
			{
				//GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Red, TEXT("bFacingUnitEnemy "));
			}
		}
		else
		{
			//GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Red, TEXT("bCanRotateToEnemy "));
		}
	}



}

void AValoriaCharacter::MoveToLocation(const FVector loc, bool canWork, ABuilding* building, AResourceMaster* resource, bool canKillAI, AActor* AIRef, bool canDestroy)
{
	if (bDied)return;


	if (building && !canDestroy)
	{

		if (!building->bConstructionIsBuilt && building->buildingWorkPointsIndex < building->buildingMaxWorker)
		{
			locationToWork = building->GetActorLocation();
		}
		else
		{
			return;
		}
	}


	if (resource)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("resource"));
		if (resource->buildingWorkPointsIndex < resource->buildingWorkPoints.Num())
		{
			locationToWork = resource->GetActorLocation();
		}
		else
		{
			return;
		}
	}

	if (canWork)
	{
		if (building)
		{
			bCanCheckForStartWork = true;
			buildingRef = building;
		}
		if (resource)
		{
			bCanCheckForStartWork = true;
			resourceRef = resource;
		}
	}


	tempLocation = loc;



	AAIController* DefaultAIController = Cast<AAIController>(GetController());
	if (DefaultAIController)
	{
		if (canWork)
		{
			DefaultAIController->MoveToLocation(locationToWork);
		}
		else if (canKillAI && AIRef && bCanAttack)
		{
			AIToAttackRef = AIRef;
			bCanCheckDistanceWithAI = true;
			DefaultAIController->MoveToLocation(AIToAttackRef->GetActorLocation());
		}
		else if (!canKillAI && !canDestroy)
		{
			FVector finalLocationToMove = loc;
			finalLocationToMove.X += UKismetMathLibrary::RandomFloatInRange(20.f, 700.f);
			finalLocationToMove.Y += UKismetMathLibrary::RandomFloatInRange(20.f, 700.f);
			DefaultAIController->MoveToLocation(finalLocationToMove);
		}
		else if (canDestroy)
		{
			buildingRef = building;
			if (buildingRef && !bIsAttackingUnits)
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("buildingRef is VALID"));
				FVector finalLocationToMove = buildingRef->GetActorLocation();

				finalLocationToMove.X += UKismetMathLibrary::RandomFloatInRange(15.f, 200.f);
				finalLocationToMove.Y += UKismetMathLibrary::RandomFloatInRange(15.f, 200.f);
				DefaultAIController->MoveToLocation(finalLocationToMove);
				//DefaultAIController->MoveToActor(buildingRef);
				bCanCheckForStartDestroy = true;
				bCanRotateToBuilding = true;
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("buildingRef is NOT VALID "));
			}

		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("DefaultAIController NOT VALID"));
	}
}

void AValoriaCharacter::RotateToBuilding(float deltaTime)
{
	if (buildingRef == nullptr || !bCanRotateToBuilding)return;

	bCanRotateToEnemy = false;
	bCanAttackBaseAgain = false;
	FRotator actorRotation = GetActorRotation();
	FVector actorLocation = GetActorLocation();
	FRotator FindLookAtRotationOutput = UKismetMathLibrary::FindLookAtRotation(actorLocation, buildingRef->GetActorLocation());
	FRotator RInterpToOutput = FMath::RInterpTo(actorRotation, FindLookAtRotationOutput, deltaTime, 15.f);
	SetActorRotation(FRotator(0.f, FindLookAtRotationOutput.Yaw, 0.f));
}

void AValoriaCharacter::RotateToResource(float deltaTime)
{
	if (resourceRef == nullptr || !bCanRotateToBuilding)return;

	FRotator actorRotation = GetActorRotation();
	FVector actorLocation = GetActorLocation();
	FRotator FindLookAtRotationOutput = UKismetMathLibrary::FindLookAtRotation(actorLocation, resourceRef->GetActorLocation());
	FRotator RInterpToOutput = FMath::RInterpTo(actorRotation, FindLookAtRotationOutput, deltaTime, 15.f);
	SetActorRotation(FRotator(0.f, FindLookAtRotationOutput.Yaw, 0.f));
}

void AValoriaCharacter::RotateToEnemy(float deltaTime)
{
	if (AIToAttackRef == nullptr || !bCanRotateToEnemy)return;

	bCanAttackBaseAgain = true;
	FRotator actorRotation = GetActorRotation();
	FVector actorLocation = GetActorLocation();
	FRotator FindLookAtRotationOutput = UKismetMathLibrary::FindLookAtRotation(actorLocation, AIToAttackRef->GetActorLocation());
	FRotator RInterpToOutput = FMath::RInterpTo(actorRotation, FindLookAtRotationOutput, deltaTime, 15.f);
	SetActorRotation(FRotator(0.f, FindLookAtRotationOutput.Yaw, 0.f));
}


void AValoriaCharacter::StopWorkAnimation()
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && BuildingAnimation)
	{
		animInstance->Montage_Stop(0.0, BuildingAnimation);
		if (GetMesh())
		{
			GetMesh()->SetRenderCustomDepth(false);
		}

	}
	if (buildingRef && buildingRef->buidlingWorkers.Num() <= 0)
	{
		for (auto Worker : buildingRef->buidlingWorkers)
		{
			UAnimInstance* animIns = Worker->GetMesh()->GetAnimInstance();
			if (animIns)
			{
				animIns->Montage_Stop(0.0, BuildingAnimation);
				if (GetMesh())
				{
					GetMesh()->SetRenderCustomDepth(false);
				}
			}

		}
	}

}


void AValoriaCharacter::CheckCharacterDistanceWithAI()
{
	if (AIToAttackRef)
	{
		float distance = AIToAttackRef->GetDistanceTo(this);
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::FromInt(distance));
		if (distance <= distanceValue)
		{
			//bCanCheckDistanceWithAI = false;
			GEngine->AddOnScreenDebugMessage(-1, 0.02f, FColor::Yellow, TEXT("the player is near of the enemy"));
			Attack();
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.02f, FColor::Yellow, TEXT("the player is far away from Enemy"));
		}
	}
}

void AValoriaCharacter::CheckCharacterDistanceWithBuildingToDestroy()
{
	if (buildingRef)
	{
		float distance = buildingRef->GetDistanceTo(this);
		GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Orange, FString::FromInt(distance));
		if (distance <= 1500.f)
		{
			bCanRotateToEnemy = false;
			DestroyBuilding();
		}
	}
}

void AValoriaCharacter::Attack()
{
	AValoriaCharacter* AItoKill = Cast<AValoriaCharacter>(AIToAttackRef);
	if (attackAnimationMontage && AItoKill && AItoKill->enemyStatus == EAIStatus::enemy)
	{
		UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
		if (animInstance)
		{
			if (AItoKill->health > 0)
			{
				if (!bIsAttackingUnits && !animInstance->IsAnyMontagePlaying())
				{
					//GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Green, TEXT("CAN KILL AI"));
					bIsAttackingUnits = true;
					bFacingUnitEnemy = true;
					bCanRotateToEnemy = false;
					animInstance->Montage_Play(attackAnimationMontage);
				}
			}
			else
			{
				bIsAttackingUnits = false;
				if (bInWarState && unitAIOwner && unitAIOwner->baseToAttack && bCanAttackBaseAgain && !bIsAttackingUnits)
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("STOPED BUT CAN ATTACK TO BASE AGAIN"));
					MoveToLocation(unitAIOwner->baseToAttack->GetActorLocation(), false, unitAIOwner->baseToAttack, nullptr, false, nullptr, true);
					bCanAttackBaseAgain = false;
				}
			}
		}
	}
}

void AValoriaCharacter::DestroyBuilding()
{
	if(ActorHasTag("Player"))
	{
		if(!buildingRef || !buildingRef->valoriaAIRef || buildingRef->valoriaAIRef->enemyStatus != EAIStatus::enemy)return;
	}

	if (attackAnimationMontage && buildingRef) // I need add this in condition---> && buildingRef->valoriaAIRef->enemyStatus == EAIStatus::enemy
	{
		//if(!ActorHasTag("Player") && !bIsAttackingUnits)return;
		
		UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
		if (animInstance)
		{
			if (!animInstance->IsAnyMontagePlaying())
			{
				if (buildingRef->GetconstructionCounter() > 0)
				{
					animInstance->Montage_Play(attackAnimationMontage);
					if (damageBaseSound)
					{
						UGameplayStatics::SpawnSoundAtLocation(this, damageBaseSound, GetActorLocation());
					}
					buildingRef->DamageBuilding(damagePower);
				}
				else
				{
					bCanCheckForStartDestroy = false;

					if (buildingRef)
					{
						if (buildingRef->destroyBaseSound)
						{
							UGameplayStatics::SpawnSoundAtLocation(this, buildingRef->destroyBaseSound, GetActorLocation());
						}
						if (buildingRef->destroyBaseParticle)
						{
							UGameplayStatics::SpawnEmitterAtLocation(this, buildingRef->destroyBaseParticle, buildingRef->GetActorLocation());
						}
						if (buildingRef->valoriaAIRef)
						{
							buildingRef->valoriaAIRef->bHasBarracks = false;
						}
						buildingRef->Destroy();
						buildingRef = nullptr;
						animInstance->Montage_Stop(0.f, attackAnimationMontage);
					}
				}
			}
		}
	}
}

void AValoriaCharacter::CheckAllNearEnemies()
{
	if(bMustAttackBase) return;
	if (!bCanAttack)return;
	//if (bCanCheckDistanceWithAI) return;
	//if (bCanRotateToEnemy)return;
	//if(bIsSelected)return;
	if (bRunAway) return;

	//GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Purple, TEXT("CheckAllNearEnemies"));
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AValoriaCharacter::StaticClass(), AllEnemies);
	for (AActor* enemy : AllEnemies)
	{
		AValoriaCharacter* newEnemy = Cast<AValoriaCharacter>(enemy);
		if (newEnemy && newEnemy != this && newEnemy->capitalCode != capitalCode && !newEnemy->bDied && newEnemy->GetDistanceTo(this) < 2500.f)
		{
			AAIController* DefaultAIController = Cast<AAIController>(GetController());
			if (ActorHasTag("Player"))
			{
				if (DefaultAIController && newEnemy->enemyStatus == EAIStatus::enemy)
				{
					DefaultAIController->MoveToLocation(newEnemy->GetActorLocation(), 100.f);
					bCanRotateToEnemy = true;
					bCanCheckDistanceWithAI = true;
					AIToAttackRef = newEnemy;
				}
			}
			else
			{
				if (DefaultAIController)
				{
					for (auto code : enemyUnitCodeToAttack)
					{
						if (code == 1) // 1 is for player
						{
							DefaultAIController->MoveToLocation(newEnemy->GetActorLocation(), 100.f);
							bCanRotateToEnemy = true;
							bCanCheckDistanceWithAI = true;
							AIToAttackRef = newEnemy;
							break;
						}
					}
				}
			}

		}
		else
		{
			AAIController* DefaultAIController = Cast<AAIController>(GetController());
			if (DefaultAIController && !ActorHasTag("Player") && !bMustAttackBase && buildingRef)
			{
				DefaultAIController->MoveToLocation(buildingRef->GetActorLocation());
			}
		}
	}
}

void AValoriaCharacter::WeaponBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this)
	{
		AValoriaCharacter* overlapedCharacter = Cast<AValoriaCharacter>(OtherActor);
		if (overlapedCharacter && overlapedCharacter->GetCapitalCode() != capitalCode)
		{
			UGameplayStatics::ApplyDamage(overlapedCharacter, damagePower, GetController(), this, UDamageType::StaticClass());
			if (fightSound)
			{
				UGameplayStatics::SpawnSoundAtLocation(this, fightSound, GetActorLocation());
			}
		}
	}
}

void AValoriaCharacter::EnemyDetectorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//if (OtherActor && OtherActor != this)
	//{
	//	AValoriaCharacter* overlapedCharacter = Cast<AValoriaCharacter>(OtherActor);
	//	if (overlapedCharacter && overlapedCharacter->GetCapitalCode() != capitalCode)
	//	{
	//		AAIController* DefaultAIController = Cast<AAIController>(GetController());
	//		if (DefaultAIController)
	//		{
	//			DefaultAIController->MoveToLocation(overlapedCharacter->GetActorLocation(),100.f);
	//		}
	//		bCanRotateToEnemy = true;
	//		bCanCheckDistanceWithAI = true;
	//		AIToAttackRef = overlapedCharacter;
	//	}
	//}
}

float AValoriaCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	attacker = Cast<AValoriaCharacter>(DamageCauser);
	bCanRotateToEnemy = true;
	if (attacker && bCanAttack)
	{
		bCanCheckDistanceWithAI = true;
		if (AIToAttackRef == nullptr)
		{
			AIToAttackRef = attacker;
			bMustAttackBase = false;
		}

	}
	bRunAway = false;
	float damagedToApplied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	damagedToApplied = FMath::Min(health, damagedToApplied);
	health -= damagedToApplied;
	WeaponCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (health <= 0)
	{
		death();
		health = 0;
		if (attacker)
		{
			attacker->AIToAttackRef = nullptr;
			attacker->attacker = nullptr;
			attacker->SetCanRotateToEnemy(false);
			attacker->SetCanCheckDistanceWithAI(false);
		}
	}
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, FString::FromInt(health));
	return health;

}

void AValoriaCharacter::death()
{
	if (diedSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, diedSound, GetActorLocation());
	}
	bDied = true;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	enemyDetector->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && deathAnimationMontage)
	{
		animInstance->Montage_Play(deathAnimationMontage);
		Tags.Empty();
		SetLifeSpan(2.f);
	}
}

void AValoriaCharacter::OnSeePawn(APawn* Pawn)
{
	if (bDied)
	{
		AAIController* DefaultAIController = Cast<AAIController>(GetController());
		if (DefaultAIController)
		{
			DefaultAIController->StopMovement();
			GetCharacterMovement()->StopActiveMovement();
			bCanRotateToEnemy = false;
			bCanCheckDistanceWithAI = false;
			AIToAttackRef = nullptr;
			return;
		}
	}

	if(bMustAttackBase)return;

	if (GetVelocity().Length() > 5.f) return;

	if (ActorHasTag("Player"))
	{
		AValoriaCharacter* seenPawn = Cast<AValoriaCharacter>(Pawn);
		if (seenPawn && seenPawn->health > 0 && seenPawn->enemyStatus == EAIStatus::enemy && seenPawn->GetCapitalCode() != capitalCode && AIToAttackRef == nullptr)
		{
			AAIController* DefaultAIController = Cast<AAIController>(GetController());

			if (DefaultAIController)
			{
				DefaultAIController->MoveToLocation(seenPawn->GetActorLocation(), distanceValue);
			}
			
			bCanRotateToEnemy = true;
			bCanCheckDistanceWithAI = true;
			AIToAttackRef = seenPawn;
			


			FLatentActionInfo latentActionInfo;
			latentActionInfo.Linkage = 0;
			latentActionInfo.CallbackTarget = this;
			latentActionInfo.ExecutionFunction = "OnCoolDownCheckSeenEnemy";
			latentActionInfo.UUID = 53344322;
			UKismetSystemLibrary::RetriggerableDelay(this, 0.6f, latentActionInfo);
		}
	}
	else
	{

		bool bAICanAttack = false;
		AValoriaCharacter* seenPawn = Cast<AValoriaCharacter>(Pawn);
		if (seenPawn && seenPawn->health > 0 && seenPawn->GetCapitalCode() != capitalCode && AIToAttackRef == nullptr)
		{
			AAIController* DefaultAIController = Cast<AAIController>(GetController());

			for (auto code : enemyUnitCodeToAttack)
			{
				if (code == 1) // 1 is for player
				{
					DefaultAIController->MoveToLocation(seenPawn->GetActorLocation(), 100.f);
					bCanRotateToEnemy = true;
					bCanCheckDistanceWithAI = true;
					AIToAttackRef = seenPawn;
					bAICanAttack = true;
					
				}
			}

			if (bAICanAttack)
			{
				if (DefaultAIController)
				{
					DefaultAIController->MoveToLocation(seenPawn->GetActorLocation(), distanceValue);
				}
				
				bCanAttack = true;
				bCanRotateToEnemy = true;
				bCanCheckDistanceWithAI = true;
				AIToAttackRef = seenPawn;
				

			}

			FLatentActionInfo latentActionInfo;
			latentActionInfo.Linkage = 0;
			latentActionInfo.CallbackTarget = this;
			latentActionInfo.ExecutionFunction = "OnCoolDownCheckSeenEnemy";
			latentActionInfo.UUID = 53344322;
			UKismetSystemLibrary::RetriggerableDelay(this, 0.6f, latentActionInfo);
		}
	}



}

void AValoriaCharacter::OnCoolDownCheckSeenEnemy()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, TEXT("OnCoolDownCheckSeenEnemy"));
	bCanRotateToEnemy = false;
	bCanCheckDistanceWithAI = false;
	AIToAttackRef = nullptr;
	if (bInWarState && unitAIOwner && unitAIOwner->baseToAttack && bCanAttackBaseAgain && !bIsAttackingUnits)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Blue, TEXT("ATTACK TO BASE AGAIN FROM COOL DOWN"));
		MoveToLocation(unitAIOwner->baseToAttack->GetActorLocation(), false, unitAIOwner->baseToAttack, nullptr, false, nullptr, true);
		bCanAttackBaseAgain = false;
		bFacingUnitEnemy = false;
	}
}
