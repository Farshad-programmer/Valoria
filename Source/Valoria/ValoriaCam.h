// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ValoriaCam.generated.h"

/** Forward declaration to improve compiling times */
class UNiagaraSystem;
class AValoriaCharacter;
class AValoriaPlayerController;
class AValoriaHUD;
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
	class UInputAction* SelectClickAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* Deselect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* SetDestinationClickAction2;

	UPROPERTY()
	TArray<AValoriaCharacter*>players;


	void DeselectAllCharacters();




	// blueprint callable functions


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Input handlers for SetDestination action. */
	void OnInputStarted();
	void OnSelectStarted();
	void OnSelectReleased();
	void OnDeselectStarted();
	void OnSetDestinationStarted2();
	void OnSetDestinationReleased2();

private:
	FVector CachedDestination;
	FHitResult Hit;
	float FollowTime; // For how long it has been pressed

	bool bIsPlayerSelected{false};

	UPROPERTY()
	TArray<AActor*> characters;


	
	bool bIsLeftMousePressed{false};

	UPROPERTY()
	AValoriaPlayerController* valoriaPlayerController;

	UPROPERTY()
	APlayerController* playerController;

	bool bMarqueeSelected{false};
	bool bCanMarqueeMove{false};
	bool bCourserHitSuccessful;

public:	
		// All getter and setter here
	FORCEINLINE bool GetIsPlayerSelected()const {return bIsPlayerSelected;}


	FORCEINLINE void SetIsPlayerSelected(bool isSelected){bIsPlayerSelected = isSelected;}
	FORCEINLINE void SetIsMarqueeSelected(bool isMarqueeSelected){bMarqueeSelected = isMarqueeSelected;}
	FORCEINLINE void SetCanMarqueeMove(bool canMarqueeMove){bCanMarqueeMove = canMarqueeMove;}

};
