// Copyright Epic Games, Inc. All Rights Reserved.

#include "ValoriaGameMode.h"
#include "ValoriaPlayerController.h"
#include "ValoriaCharacter.h"
#include "UObject/ConstructorHelpers.h"

AValoriaGameMode::AValoriaGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AValoriaPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownCharacter"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set default controller to our Blueprinted controller
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/TopDown/Blueprints/BP_TopDownPlayerController"));
	if(PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}
}