// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "GameFramework/Character.h"
#include "ValoriaCharacter.generated.h"

class UNiagaraComponent;
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

	void MoveToLocation(const FVector loc);

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Efect, meta = (AllowPrivateAccess = "true"))
	class UNiagaraComponent* SelectionNiagara;

	TArray<AActor*> characters;


public:
	FORCEINLINE void SetSelectionNiagaraVisibility(bool makeVisible){SelectionNiagara->SetVisibility(makeVisible);}
};

