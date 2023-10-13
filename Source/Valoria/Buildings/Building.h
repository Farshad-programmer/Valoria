// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Building.generated.h"

class AValoriaCam;
UCLASS()
class VALORIA_API ABuilding : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* BuildingMesh;



public:	
	ABuilding();
	virtual void Tick(float DeltaTime) override;

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
