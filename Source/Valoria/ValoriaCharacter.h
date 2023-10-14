// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "GameFramework/Character.h"
#include "ValoriaCharacter.generated.h"

class UNiagaraComponent;
class ABuilding;
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

	void MoveToLocation(const FVector loc,bool canWork,ABuilding* building);
	void StartWork();
	void CheckCharacterDistanceWithBuilding();
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

	// animations
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Animations, meta=(AllowPrivateAccess = "true"))
	class UAnimMontage* BuildingAnimation;


	TArray<AActor*> characters;

	bool bCanCheckForStartWork{false};
	bool bIsStartedWork{false};

	bool bHasProblemToFindDistanceWithBuilding{false};
	int32 workerIssueCounter {0};

	FVector locationToWork;
	FVector tempLocation;
	
	float distanceValue = 400.f;
	void RotateToBuilding(float deltaTime);
	bool bCanRotateToBuilding{true};


	void ResetWorker();

public:
	FORCEINLINE bool GetIsStartedWork()const {return bIsStartedWork;}




	FORCEINLINE void SetSelectionNiagaraVisibility(bool makeVisible){SelectionNiagara->SetVisibility(makeVisible);}
	FORCEINLINE void SetCheckForStartWork(bool canCheck){bCanCheckForStartWork = canCheck;}
	FORCEINLINE void SetIsStartedWork(bool isStarted){bIsStartedWork = isStarted;}
};


