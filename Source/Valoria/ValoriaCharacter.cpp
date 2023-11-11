// Copyright Epic Games, Inc. All Rights Reserved.

#include "ValoriaCharacter.h"
#include "ValoriaCam.h"

#include "AIController.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
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

	Weapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon"));
	Weapon->SetupAttachment(GetMesh(), "RightHandWeaponSocket");
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Collider"));
	WeaponCollider->SetupAttachment(GetMesh(), "RightHandWeaponSocket");
	WeaponCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	SelectionNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SelectionNiagara"));
	SelectionNiagara->SetupAttachment(RootComponent);
	SelectionNiagara->SetVisibility(false);

	Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Overlay Widget"));
	Widget->SetupAttachment(GetMesh());
	Widget->SetVisibility(false);

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}




void AValoriaCharacter::BeginPlay()
{
	Super::BeginPlay();
	health = maxHealth;
	WeaponCollider->OnComponentBeginOverlap.AddDynamic(this, &AValoriaCharacter::WeaponBeginOverlap);

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
		//RotateToEnemy(DeltaSeconds);
	}
}

void AValoriaCharacter::MoveToLocation(const FVector loc, bool canWork, ABuilding* building, AResourceMaster* resource, bool canKillAI, AActor* AIRef)
{

	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Yellow, FString::Printf(TEXT("canKillAI: %s"), canKillAI ? TEXT("true") : TEXT("false")));


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
	if (AIToAttackRef)
	{
		float distance = AIToAttackRef->GetDistanceTo(this);
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::FromInt(distance));
		if (distance <= 250.f)
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

void AValoriaCharacter::Attack()
{
	AValoriaCharacter* AItoKill = Cast<AValoriaCharacter>(AIToAttackRef);
	if (attackAnimationMontage && AItoKill)
	{
		UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
		if (animInstance)
		{
			if (AItoKill->health > 0 && !bIsAttacking && !animInstance->IsAnyMontagePlaying())
			{
				GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Attacking"));
				bIsAttacking = true;
				animInstance->Montage_Play(attackAnimationMontage);
			}
		}
	}
}

void AValoriaCharacter::WeaponBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && OtherActor->ActorHasTag("AI"))
	{
		AValoriaCharacter* overlapedCharacter = Cast<AValoriaCharacter>(OtherActor);
		if (overlapedCharacter)
		{
			UGameplayStatics::ApplyDamage(overlapedCharacter, 50.f, GetController(), this, UDamageType::StaticClass());
		}
	}
}

float AValoriaCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	attacker = Cast<AValoriaCharacter>(DamageCauser);
	bCanRotateToEnemy = true;
	if (attacker)
	{
		bCanCheckDistanceWithAI = true;
		AIToAttackRef = attacker;
	}

	float damagedToApplied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	damagedToApplied = FMath::Min(health, damagedToApplied);
	health -= damagedToApplied;
	if (health <= 0)
	{
		death();
		health = 0;
	}
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, FString::FromInt(health));
	return health;

}

void AValoriaCharacter::death()
{
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (animInstance && deathAnimationMontage)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("death"));
		animInstance->Montage_Play(deathAnimationMontage);
		Tags.Empty();
		SetLifeSpan(7.f);
	}
}
