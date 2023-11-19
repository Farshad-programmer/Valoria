// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Valoria/Additives.h"
#include "AValoriaAI.generated.h"

class ACityCenter;
class AValoriaWorker;
class ABarracks;
class AMapBorder;
class AValoriaInfantry;



UCLASS()
class VALORIA_API AValoriaAI : public AActor
{
	GENERATED_BODY()
	
public:	
	AValoriaAI();

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category= Game, meta=(AllowPrivateAccess = "true"))
	EAIStatus enemyStatus;

	FString tag;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category= Game, meta=(AllowPrivateAccess = "true"))
	int32 capitalCode;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Game, meta=(AllowPrivateAccess = "true"))
	TSubclassOf<ABarracks> barracksClass;

	bool bHasBarracks{false};

	TArray<FVector> barracksLocation ;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Game, meta=(AllowPrivateAccess = "true"))
	FString capitalName;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Game, meta=(AllowPrivateAccess = "true"))
	int32 infantryNumber = 0;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Game, meta=(AllowPrivateAccess = "true"))
	int32 gold = 20;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Game, meta=(AllowPrivateAccess = "true"))
	int32 stone = 50;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Game, meta=(AllowPrivateAccess = "true"))
	int32 wood = 100;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Game, meta=(AllowPrivateAccess = "true"))
	int32 science = 20;

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


	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category= Game, meta=(AllowPrivateAccess = "true"))
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


	UFUNCTION()
	void InitialAIStatus();

	void FindAPlaceForMakingBarracksforAI();

	void UpdateAIUnits();


public:	
	virtual void Tick(float DeltaTime) override;

};
