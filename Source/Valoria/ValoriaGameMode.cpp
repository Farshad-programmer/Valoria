// Copyright Epic Games, Inc. All Rights Reserved.

#include "ValoriaGameMode.h"
#include "ValoriaPlayerController.h"
#include "ValoriaCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Valoria/AI/EnemyAI.h"

AValoriaGameMode::AValoriaGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AValoriaPlayerController::StaticClass();

	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Player/BP_Valoria"));
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	// set default controller to our Blueprinted controller
	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(TEXT("/Game/Blueprints/Player/BP_ValoriaPlayerController"));
	if (PlayerControllerBPClass.Class != NULL)
	{
		PlayerControllerClass = PlayerControllerBPClass.Class;
	}

	numberOfEnemies = 2;
}

void AValoriaGameMode::BeginPlay()
{
	Super::BeginPlay();

	for (int i = 0; i < 2; ++i)
	{
		if (GetWorld() && enemyAIClass)
		{
			GetWorld()->SpawnActor<AEnemyAI>(enemyAIClass, FVector(0.f), FRotator(0.f));
		}
	}


}
