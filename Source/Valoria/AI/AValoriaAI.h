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
class AValoriaCam;


UCLASS()
class VALORIA_API AValoriaAI : public AActor
{
	GENERATED_BODY()



public:

	// public functions
	AValoriaAI();
	virtual void Tick(float DeltaTime) override;
	void SpawnAIFirstCityCenter();
	void SpawnAIFirstWorkers();


	// public variables

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category= Game, meta=(AllowPrivateAccess = "true"))
	EAIStatus enemyStatus;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category= Game, meta=(AllowPrivateAccess = "true"))
	int32 capitalCode;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Game, meta=(AllowPrivateAccess = "true"))
	TSubclassOf<ABarracks> barracksClass;

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

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Game, meta=(AllowPrivateAccess = "true"))
	TArray<int32>enemiesCode;

	UPROPERTY()
	ACityCenter* baseToAttack;


	TArray<FVector> barracksLocation ;
	int32 capitalToAttack = 0;
	int32 attackCounter {2}; // To determine the target of the attack = some enemies only attack bases and some attack soldiers 
	FString tag;
	int32 woodUpgradeRate{1};
	int32 stoneUpgradeRate{1};
	int32 goldUpgradeRate{1};
	int32 scieneUpgradeRate{1};
	int32 AIRank{1};
	int32 AIAllUnitNumber = 0;

protected:
	virtual void BeginPlay() override;


private:

	// private variables
	FVector AIBaseCenterLocation ;
	uint32 baseUnit{0};
	uint32 unitInCapital{0};

	bool bHasBarracks{false};
	bool bInWarState{false};
	bool bBaseToAttackIdentified{false};
	bool bOrderUnitsToAttackBase{false};
	bool bOrderUnitsToAttackUnits{false};
	bool bAskGold{false};
	bool bAskStone{false};
	bool bAskWood{false};
	bool bAskScience{false};
	bool bNeedAction{false};
	// Subclasses

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Game, meta=(AllowPrivateAccess = "true"))
	TSubclassOf<ACityCenter>cityCenterClass;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Game, meta=(AllowPrivateAccess = "true"))
	TSubclassOf<AValoriaWorker> valoriaWorkerClass;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Game, meta=(AllowPrivateAccess = "true"))
	TSubclassOf<AValoriaInfantry> valoriaInfantryClass;

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category= Game, meta=(AllowPrivateAccess = "true"))
	AMapBorder* mapBorderRef;

	UPROPERTY()
	AValoriaWorker* Spawnedworker;

	UPROPERTY()
	AValoriaCam* playerRef ;
	// Timer handles

	FTimerHandle moveAITimerhandler;
	FTimerHandle spawnSoldierTimerHandle;

	
	// Private Functions

	void CheckWarState();
	void FindNearestBaseToAttack();
	void OrderSoldiersToAttack(int32 enemyCapitalCodeToAttack);
	void CheckingUnitNumberInBase();
	void FindAPlaceForMakingBarracksforAI();
	void UpdateAIUnits();
	void UpdateAIStuff();
	void ManageAIActions();
	void InspectPlayerForces();
	void InspectPlayerResources();
	UFUNCTION()
	void InitialAIStatus();

	UFUNCTION()
	void AIMoveToBuilding();

	UFUNCTION()
	void SpawnSoldier();


public:	
	// Getters
	FORCEINLINE bool GetHasBarracks()const {return bHasBarracks;}


	//Setters
	FORCEINLINE void SetHasBarracks(bool has){bHasBarracks = has;}
};

