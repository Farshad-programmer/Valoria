// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildingBanner.generated.h"
class ABuilding;
UCLASS()
class VALORIA_API ABuildingBanner : public AActor
{
	GENERATED_BODY()
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* BannerMesh;

public:	
	ABuildingBanner();
	virtual void Tick(float DeltaTime) override;

	bool bBannerAdjusted{false};

	UPROPERTY()
	ABuilding* buildingRelated;

protected:
	virtual void BeginPlay() override;

public:	

};
