// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ResourceMaster.generated.h"
class AValoriaCam;
class UStaticMesh;
class UWidgetComponent;
class AValoriaCharacter;

UENUM(BlueprintType)
enum class EResourceType:uint8
{
	wood,
	stone,
	gold,
	science
};

UCLASS()
class VALORIA_API AResourceMaster : public AActor
{
	GENERATED_BODY()

public:
	// public functions
	virtual void Tick(float DeltaTime) override;

	// public variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	int32 workerNumber{0};

	UPROPERTY()
	TArray<AValoriaCharacter*>ResourceWorkers;

	UPROPERTY()
	AValoriaCam* valoriaCam;

	UPROPERTY()
	TArray<AValoriaCharacter*>buidlingWorkers;

	
	bool bWorkProgressStarted{false};
	float workProgressSpeed{40.f};
	TArray<FVector>buildingWorkPoints;
	int32 buildingWorkPointsIndex;

private:
	// Private Functions
	AResourceMaster();

	// private variables

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* ResourceMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WorkerPoint1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WorkerPoint2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WorkerPoint3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* Widget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details,meta = (AllowPrivateAccess = "true"))
	float workersStartWorkDistance = {400.f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float resourceFinishValue = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float resourceCounter{0};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	EResourceType resourceType;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int resourceIncomeDelay = 300;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int resourceIncomeValue = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	int resourseIncomePower = 4;

	float resourcePlusCounter;



protected:
	virtual void BeginPlay() override;
	void UpdateResources(float DeltaTime);


public:

	// Getter
	FORCEINLINE float GetWorkersStartWorkDistance()const {return workersStartWorkDistance;}


	// Setter

};
