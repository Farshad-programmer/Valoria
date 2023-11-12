// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Character.h"
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
	AValoriaCharacter();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	UPROPERTY()
	ABuilding* buildingRef;

	UPROPERTY()
	AResourceMaster* resourceRef;

	void MoveToLocation(const FVector loc,bool canWork,ABuilding* building = nullptr,AResourceMaster* resource = nullptr,bool canKillAI = false,AActor* AIRef = nullptr);

	void StopWorkAnimation();



private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Efect, meta = (AllowPrivateAccess = "true"))
	class UNiagaraComponent* SelectionNiagara;

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

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= stat, meta=(AllowPrivateAccess = "true"))
	float distanceValue = 400.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= stat, meta=(AllowPrivateAccess = "true"))
	float health ;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= stat, meta=(AllowPrivateAccess = "true"))
	float maxHealth = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= stat, meta=(AllowPrivateAccess = "true"))
	float damagePower{50.f};
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess = "true"))
	bool bIsAttacking{false};

	bool bDied{false};

	TArray<AActor*> characters;



	bool bHasProblemToFindDistanceWithBuilding{false};
	int32 workerIssueCounter {0};

	FVector locationToWork;
	FVector tempLocation;


	void RotateToBuilding(float deltaTime);
	void RotateToResource(float deltaTime);
	void RotateToEnemy(float deltaTime);
	bool bCanRotateToBuilding{true};
	bool bCanCheckDistanceWithAI{false};
	bool bCanRotateToEnemy{false};
	bool bIsSelected{false};
	bool bRunAway{false};
	int32 capitalCode ;

	UPROPERTY()
	AActor* AIToAttackRef;

	UPROPERTY()
	AValoriaCharacter* attacker;

	void CheckCharacterDistanceWithAI();
	void Attack();


	TArray<AActor*> AllEnemies;
	void CheckAllNearEnemies();


	UFUNCTION()
	void WeaponBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);

	UFUNCTION()
	void EnemyDetectorBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);



	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void death();


	// Pawn Sensing
	UFUNCTION()
	void OnSeePawn(APawn* Pawn);


	//Timers
	UFUNCTION()
	void OnCoolDownCheckSeenEnemy();

protected:
	virtual void BeginPlay() override;
	bool bIsStartedWork{false};
	bool bCanCheckForStartWork{false};
	bool bCanAttack{true};
	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	USphereComponent* enemyDetector;

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Solider Class", meta = (AllowPrivateAccess = "true"))
	ESoliderClass soliderClass;

	// animations
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Animations, meta=(AllowPrivateAccess = "true"))
	class UAnimMontage* BuildingAnimation;

	


public:
	FORCEINLINE bool GetIsStartedWork()const {return bIsStartedWork;}
	FORCEINLINE UWidgetComponent* GetOverlayWidget()const {return Widget;}
	FORCEINLINE int32 GetCapitalCode()const {return capitalCode;}


	FORCEINLINE void SetRunAway(bool runAway){bRunAway = runAway;}
	FORCEINLINE void SetIsSelected(bool selected){bIsSelected = selected;}
	FORCEINLINE void SetSelectionNiagaraVisibility(bool makeVisible){SelectionNiagara->SetVisibility(makeVisible);}
	FORCEINLINE void SetCheckForStartWork(bool canCheck){bCanCheckForStartWork = canCheck;}
	FORCEINLINE void SetIsStartedWork(bool isStarted){bIsStartedWork = isStarted;}
	FORCEINLINE void SetOverlayWidgetVisibility(bool bShow){Widget->SetVisibility(bShow);}
	FORCEINLINE void SetCapitalCode(int32 newCode){capitalCode = newCode;}
	FORCEINLINE void SetCanCheckDistanceWithAI(bool canCheck){bCanCheckDistanceWithAI = canCheck;}
	FORCEINLINE void SetCanRotateToEnemy(bool canRotate){bCanRotateToEnemy = canRotate;}
};






