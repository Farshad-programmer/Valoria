// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemyAI.generated.h"

class ACityCenter;
class AValoriaWorker;
UCLASS()
class VALORIA_API AEnemyAI : public AActor
{
	GENERATED_BODY()
	
public:	
	AEnemyAI();

protected:
	virtual void BeginPlay() override;


private:

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Game, meta=(AllowPrivateAccess = "true"))
	TSubclassOf<ACityCenter>cityCenterClass;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Game, meta=(AllowPrivateAccess = "true"))
	TSubclassOf<AValoriaWorker> valoriaWorkerClass;

	FVector AIBaseCenterLocation ;

public:	
	virtual void Tick(float DeltaTime) override;

};
