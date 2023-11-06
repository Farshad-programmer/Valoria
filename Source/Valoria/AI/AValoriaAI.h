// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AValoriaAI.generated.h"

class ACityCenter;
class AValoriaWorker;
class ABarracks;
class AMapBorder;
class AValoriaInfantry;
UENUM(BlueprintType)
enum class EEnemyStatus:uint8
{
	ally,
	enemy
};


UCLASS()
class VALORIA_API AValoriaAI : public AActor
{
	GENERATED_BODY()
	
public:	
	AValoriaAI();


	EEnemyStatus enemyStatus;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Game, meta=(AllowPrivateAccess = "true"))
	TSubclassOf<ABarracks> barracksClass;

	bool bHasBarracks{false};

	TArray<FVector> barracksLocation ;

protected:
	virtual void BeginPlay() override;


private:

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Game, meta=(AllowPrivateAccess = "true"))
	TSubclassOf<ACityCenter>cityCenterClass;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Game, meta=(AllowPrivateAccess = "true"))
	TSubclassOf<AValoriaWorker> valoriaWorkerClass;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Game, meta=(AllowPrivateAccess = "true"))
	TSubclassOf<AValoriaInfantry> valoriaInfantryClass;

	FVector AIBaseCenterLocation ;


	UPROPERTY()
	AMapBorder* mapBorderRef;

	UPROPERTY()
	AValoriaWorker* Spawnedworker;


	FTimerHandle moveAITimerhandler;

	UFUNCTION()
	void AIMoveToBuilding();

	FTimerHandle spawnSoldierTimerHandle;
	UFUNCTION()
	void SpawnSoldier();

	uint32 baseUnit{0};


	void FindAPlaceForMakingBarracksforAI();

public:	
	virtual void Tick(float DeltaTime) override;

};
