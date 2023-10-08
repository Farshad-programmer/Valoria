// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ValoriaCam.generated.h"

/** Forward declaration to improve compiling times */
class UNiagaraSystem;
class AValoriaCharacter;
UCLASS()
class VALORIA_API AValoriaCam : public APawn
{
	GENERATED_BODY()


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* ValeriaCamera;


public:
	// Sets default values for this pawn's properties
	AValoriaCam();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

		/** Time Threshold to know if it was a short press */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	float ShortPressThreshold;

	/** FX Class that we will spawn when clicking */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UNiagaraSystem* FXCursor;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;
	
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* SetDestinationClickAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* Deselect;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Input handlers for SetDestination action. */
	void OnInputStarted();
	void OnSetDestinationTriggered();
	void OnSetDestinationReleased();
	void OnDeselectStarted();


private:
	FVector CachedDestination;
	FHitResult Hit;
	float FollowTime; // For how long it has been pressed
	bool bIsPlayerSelected{false};
	TArray<AActor*> characters;
	AValoriaCharacter* player;

public:	
		// All getter and setter here



};
