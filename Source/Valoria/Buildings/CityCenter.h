// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Building.h"
#include "CityCenter.generated.h"

/**
 * 
 */

UCLASS()
class VALORIA_API ACityCenter : public ABuilding
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	bool bIsStarterCityCenter{false};

public:
	ACityCenter();
	void SetBuildingToStarterBuilding();
	virtual void Tick(float DeltaTime) override;
protected:
	virtual void BeginPlay() override;
private:

public:
	FORCEINLINE bool SetIsStarterCityCenter(bool isStarter){return bIsStarterCityCenter = isStarter;}

};
