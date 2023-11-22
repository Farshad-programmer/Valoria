// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Character.h"
#include "Valoria/Additives.h"
#include "ValoriaCharacter.generated.h"


class UNiagaraComponent;
class ABuilding;
class AResourceMaster;
class UWidgetComponent;
class UAnimMontage;
class UBoxComponent;
class UPawnSensingComponent;
class USphereComponent;
class USoundCue;
class UParticleSystem;
class AValoriaAI;

UENUM(BlueprintType)
enum class ESoliderClass:uint8
{
	worker,
	swordman,
	spearman,
	commander
};



UCLASS(Blueprintable)
class AValoriaCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// public functions
	AValoriaCharacter();
	virtual void Tick(float DeltaSeconds) override;
	void CheckIfNeedMoveToBuildingOrResourcePlaces(ABuilding* building, AResourceMaster* resource, bool canDestroy);
	void CheckIfCanWork(bool canWork, ABuilding* building, AResourceMaster* resource);
	void MoveToSpecificLocation(FVector loc, bool canWork, ABuilding* building, bool canKillAI, AActor* AIRef,
	                            bool canDestroy);
	void MoveToLocation(const FVector loc,bool canWork,ABuilding* building = nullptr,AResourceMaster* resource = nullptr,bool canKillAI = false,AActor* AIRef = nullptr,bool canDestroy = false);
	void StopWorkAnimation();

	// public variables

	UPROPERTY()
	ABuilding* buildingRef;

	UPROPERTY()
	AResourceMaster* resourceRef;
	
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category= Game, meta=(AllowPrivateAccess = "true"))
	EAIStatus enemyStatus;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Game, meta=(AllowPrivateAccess = "true"))
	TArray<int32>enemyUnitCodeToAttack;

	UPROPERTY()
	AValoriaAI* unitAIOwner;

	// AI
	bool bMustAttackBase{false};


private:

	// private variables********************

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Efect, meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* SelectionNiagara;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* WeaponCollider;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* Widget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UPawnSensingComponent* pawnSensing;


	// Animation montages
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation montage", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* attackAnimationMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation montage", meta = (AllowPrivateAccess = "true"))
	UAnimMontage* deathAnimationMontage;

	// sounds
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Sounds, meta = (AllowPrivateAccess = "true"))
	USoundCue* fightSound;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Sounds, meta = (AllowPrivateAccess = "true"))
	USoundCue* diedSound;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Sounds, meta = (AllowPrivateAccess = "true"))
	USoundCue* damageBaseSound;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= stat, meta=(AllowPrivateAccess = "true"))
	float distanceValue = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= stat, meta=(AllowPrivateAccess = "true"))
	float health ;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= stat, meta=(AllowPrivateAccess = "true"))
	float maxHealth = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= stat, meta=(AllowPrivateAccess = "true"))
	float damagePower{50.f};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess = "true"))
	bool bIsAttackingUnits{false};

	UPROPERTY()
	TArray<AActor*> characters;

	UPROPERTY()
	AActor* AIToAttackRef;

	UPROPERTY()
	AValoriaCharacter* attacker;

	UPROPERTY()
	TArray<AActor*> AllEnemies;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Game, meta=(AllowPrivateAccess = "true"))
	int32 capitalCode ;

	bool bFacingUnitEnemy{false};
	bool bDied{false};
	bool bInWarState{false};
	bool bCanAttackBaseAgain{false};
	bool bHasProblemToFindDistanceWithBuilding{false};
	int32 workerIssueCounter {0};
	FVector locationToWork;
	FVector tempLocation;
	bool bCanRotateToBuilding{true};
	bool bCanCheckDistanceWithAI{false};
	bool bCanRotateToEnemy{false};
	bool bIsSelected{false};
	bool bRunAway{false};

	// Private Functions
	void RotateToBuilding(float deltaTime);
	void RotateToResource(float deltaTime);
	void RotateToEnemy(float deltaTime);
	void CheckCharacterDistanceWithAI();
	void CheckCharacterDistanceWithBuildingToDestroy();
	void Attack();
	void DestroyBuilding();
	void CheckAllNearEnemies();
	void death();
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION()
	void WeaponBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void EnemyDetectorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);


	// Pawn Sensing
	UFUNCTION()
	void OnSeePawn(APawn* Pawn);

	//Timers
	UFUNCTION()
	void OnCoolDownCheckSeenEnemy();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	USphereComponent* enemyDetector;

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Solider Class", meta = (AllowPrivateAccess = "true"))
	ESoliderClass soliderClass;

	// animations
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Animations, meta=(AllowPrivateAccess = "true"))
	UAnimMontage* BuildingAnimation;

	bool bIsStartedWork{false};
	bool bCanCheckForStartWork{false};
	bool bCanCheckForStartDestroy{false};
	bool bCanAttack{true};


public:
	FORCEINLINE bool GetIsStartedWork()const {return bIsStartedWork;}
	FORCEINLINE UWidgetComponent* GetOverlayWidget()const {return Widget;}
	FORCEINLINE int32 GetCapitalCode()const {return capitalCode;}
	FORCEINLINE bool GetInWarState()const {return bInWarState;}

	FORCEINLINE void SetRunAway(bool runAway){bRunAway = runAway;}
	FORCEINLINE void SetIsSelected(bool selected){bIsSelected = selected;}
	FORCEINLINE void SetSelectionNiagaraVisibility(bool makeVisible){SelectionNiagara->SetVisibility(makeVisible);}
	FORCEINLINE void SetCheckForStartWork(bool canCheck){bCanCheckForStartWork = canCheck;}
	FORCEINLINE void SetIsStartedWork(bool isStarted){bIsStartedWork = isStarted;}
	FORCEINLINE void SetOverlayWidgetVisibility(bool bShow){Widget->SetVisibility(bShow);}
	FORCEINLINE void SetCapitalCode(int32 newCode){capitalCode = newCode;}
	FORCEINLINE void SetCanCheckDistanceWithAI(bool canCheck){bCanCheckDistanceWithAI = canCheck;}
	FORCEINLINE void SetCanRotateToEnemy(bool canRotate){bCanRotateToEnemy = canRotate;}
	FORCEINLINE void SetInWarState(bool state){bInWarState = state;}
	FORCEINLINE void SetCanRotateToBuilding(bool canRotate){bCanRotateToBuilding = canRotate;}
};






