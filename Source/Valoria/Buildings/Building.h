// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Building.generated.h"

class AValoriaCam;
class USceneComponent;
class UWidgetComponent;
class AValoriaCharacter;
UCLASS()
class VALORIA_API ABuilding : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* BuildingMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WorkerPoint1;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WorkerPoint2;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WorkerPoint3;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* Widget;

public:	
	ABuilding();
	virtual void Tick(float DeltaTime) override;


	TArray<FVector>buildingWorkPoints;
	int32 buildingWorkPointsIndex;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh)
	int32 workerNumber{0};

	UPROPERTY()
	TArray<AValoriaCharacter*>buidlingWorkers;

protected:
	virtual void BeginPlay() override;
	int32 buildingLevel {0};
	FHitResult Hit;

	UPROPERTY()
	AValoriaCam* valoriaCam;

	


	bool bIsBuildingSpawned{false};
	bool bCanPlaceBuilding{false};
public:

	FORCEINLINE bool GetCanPlaceBuilding()const {return bCanPlaceBuilding;}



	FORCEINLINE void SetIsBuildingSpawned(bool IsSpawned){bIsBuildingSpawned = IsSpawned;}
	FORCEINLINE void SetCanPlaceBuilding(bool canPlace){bCanPlaceBuilding = canPlace;}

};
