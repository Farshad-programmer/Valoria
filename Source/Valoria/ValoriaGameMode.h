// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ValoriaGameMode.generated.h"

class AEnemyAI;
UCLASS(minimalapi)
class AValoriaGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AValoriaGameMode();
	void BeginPlay() override;



private:
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category= Game, meta=(AllowPrivateAccess = "true"))
	int32 numberOfEnemies;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category= Game, meta=(AllowPrivateAccess = "true"))
	TSubclassOf<AEnemyAI>enemyAIClass;
	
};



