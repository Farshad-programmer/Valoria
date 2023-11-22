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

public:	
	ABuildingBanner();
	virtual void Tick(float DeltaTime) override;

	bool bBannerAdjusted{false};

	UPROPERTY()
	ABuilding* buildingRelated;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* BannerMesh;

protected:
	virtual void BeginPlay() override;

public:


};
