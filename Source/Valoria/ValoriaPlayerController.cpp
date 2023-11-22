// Copyright Epic Games, Inc. All Rights Reserved.

#include "ValoriaPlayerController.h"
#include "NiagaraFunctionLibrary.h"
#include "ValoriaCharacter.h"


AValoriaPlayerController::AValoriaPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

}

void AValoriaPlayerController::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	bShowMouseCursor = true;
	FInputModeGameOnly inputMode;
	SetInputMode(inputMode);
}






//GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Blue,selectedCharacterName);
