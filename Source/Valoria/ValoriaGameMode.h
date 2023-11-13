// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ValoriaGameMode.generated.h"

class AValoriaAI;
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
	TSubclassOf<AValoriaAI>enemyAIClass;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category= Game, meta=(AllowPrivateAccess = "true"))
	int32 startingAIBase = 2;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category= Game, meta=(AllowPrivateAccess = "true"))
	TArray<FString>CapitalNames;

	int32 capitalNameIndex{0};
};



