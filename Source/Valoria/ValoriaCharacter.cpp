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
#include "Valoria/Resources/ResourceMaster.h"
#include "Components/WidgetComponent.h"
#include "Animation/AnimMontage.h"
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
	}


}

void AValoriaCharacter::MoveToLocation(const FVector loc, bool canWork, ABuilding* building, AResourceMaster* resource, bool canKillAI, AActor* AIRef)
{
	if (bDied)return;

	if (building)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Try to work on Buidling"));
		if (!building->bConstructionIsBuilt && building->buildingWorkPointsIndex < building->buildingMaxWorker)
		{
			locationToWork = building->GetActorLocation();
		}
		else
		{
			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("The building currently has enough workers"));
			return;
		}
	}


	if (resource)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Try to work on Resource "));
		if (resource->buildingWorkPointsIndex < resource->buildingWorkPoints.Num())
		{
			locationToWork = resource->GetActorLocation();
		}
		else
		{
			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("The resource currently has enough workers"));
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
			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("want work"));
			DefaultAIController->MoveToLocation(locationToWork);
			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, FString::Printf(TEXT("location : %s"), *locationToWork.ToString()));
		}
		else if (canKillAI && AIRef && bCanAttack)
		{
			AIToAttackRef = AIRef;
			bCanCheckDistanceWithAI = true;
			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Ready to Kill Enemy"));
			DefaultAIController->MoveToLocation(AIRef->GetActorLocation());
		}
		else if (!canKillAI)
		{

			//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, FString::Printf(TEXT("location : %s"), *loc.ToString()));
			FVector finalLocationToMove = loc;
			finalLocationToMove.X += UKismetMathLibrary::RandomFloatInRange(20.f, 700.f);
			finalLocationToMove.Y += UKismetMathLibrary::RandomFloatInRange(20.f, 700.f);
			DefaultAIController->MoveToLocation(finalLocationToMove);

		}
	}



}

void AValoriaCharacter::RotateToBuilding(float deltaTime)
{
	if (buildingRef == nullptr || !bCanRotateToBuilding)return;

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
	if (attacker == nullptr || !bCanRotateToEnemy)return;

	FRotator actorRotation = GetActorRotation();
	FVector actorLocation = GetActorLocation();
	FRotator FindLookAtRotationOutput = UKismetMathLibrary::FindLookAtRotation(actorLocation, attacker->GetActorLocation());
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
			animIns->Montage_Stop(0.0, BuildingAnimation);
			if (GetMesh())
			{
				GetMesh()->SetRenderCustomDepth(false);
			}
		}
	}

}


void AValoriaCharacter::CheckCharacterDistanceWithAI()
{
	if (bCanAttack)
	{
		if (AIToAttackRef)
		{
			float distance = AIToAttackRef->GetDistanceTo(this);
			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::FromInt(distance));
			if (distance <= distanceValue)
			{
				//bCanCheckDistanceWithAI = false;
				//GEngine->AddOnScreenDebugMessage(-1, 0.02f, FColor::Yellow, TEXT("the player is near of the enemy"));
				Attack();
			}
			else
			{
				//GEngine->AddOnScreenDebugMessage(-1, 0.02f, FColor::Yellow, TEXT("the player is far away from Enemy"));
			}
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("bCanAttack is false"));
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
			if (AItoKill->health > 0 && !bIsAttacking && !animInstance->IsAnyMontagePlaying())
			{
				bIsAttacking = true;
				bCanRotateToEnemy = false;
				animInstance->Montage_Play(attackAnimationMontage);
			}
		}
	}
}

void AValoriaCharacter::CheckAllNearEnemies()
{
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
}
