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
private:

	AResourceMaster();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* ResourceMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WorkerPoint1;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WorkerPoint2;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WorkerPoint3;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* Widget;

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

public:

	virtual void Tick(float DeltaTime) override;
	TArray<FVector>buildingWorkPoints;
	int32 buildingWorkPointsIndex;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	int32 workerNumber{0};


	UPROPERTY()
	TArray<AValoriaCharacter*>ResourceWorkers;

	
	UPROPERTY()
	AValoriaCam* valoriaCam;

	bool bWorkProgressStarted{false};

	float workProgressSpeed{40.f};
	 

	UPROPERTY()
	TArray<AValoriaCharacter*>buidlingWorkers;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details)
	float workersStartWorkDistance = {400.f};

public:	
	FORCEINLINE float GetWorkersStartWorkDistance()const {return workersStartWorkDistance;}

};
