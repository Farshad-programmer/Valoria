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

	void MoveToLocation(const FVector loc,bool canWork,ABuilding* building = nullptr,AResourceMaster* resource = nullptr);

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* Widget;



	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= stat, meta=(AllowPrivateAccess = "true"))
	float health ;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= stat, meta=(AllowPrivateAccess = "true"))
	float maxHealth = 300.f;


	TArray<AActor*> characters;

	

	bool bHasProblemToFindDistanceWithBuilding{false};
	int32 workerIssueCounter {0};

	FVector locationToWork;
	FVector tempLocation;
	
	float distanceValue = 400.f;
	void RotateToBuilding(float deltaTime);
	void RotateToResource(float deltaTime);
	bool bCanRotateToBuilding{true};




protected:
	virtual void BeginPlay() override;
	bool bIsStartedWork{false};
	bool bCanCheckForStartWork{false};

	// animations
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Animations, meta=(AllowPrivateAccess = "true"))
	class UAnimMontage* BuildingAnimation;

	


public:
	FORCEINLINE bool GetIsStartedWork()const {return bIsStartedWork;}
	FORCEINLINE UWidgetComponent* GetOverlayWidget()const {return Widget;}



	FORCEINLINE void SetSelectionNiagaraVisibility(bool makeVisible){SelectionNiagara->SetVisibility(makeVisible);}
	FORCEINLINE void SetCheckForStartWork(bool canCheck){bCanCheckForStartWork = canCheck;}
	FORCEINLINE void SetIsStartedWork(bool isStarted){bIsStartedWork = isStarted;}
	FORCEINLINE void SetOverlayWidgetVisibility(bool bShow){Widget->SetVisibility(bShow);}
};


