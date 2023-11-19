// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Building.h"
#include "Components/TextRenderComponent.h"
#include "CityCenter.generated.h"

/**
 * 
 */

class UTextRenderComponent;

UCLASS()
class VALORIA_API ACityCenter : public ABuilding
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	bool bIsStarterCityCenter{false};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UTextRenderComponent* capitalNameTextRender;


public:
	ACityCenter();
	void SetBuildingToStarterBuilding();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent)
	void SetCapitalNameTextRender();
protected:
	virtual void BeginPlay() override;
private:

public:

	FORCEINLINE UTextRenderComponent* GetCapitalNameTextRender()const {return capitalNameTextRender;}




	FORCEINLINE void SetIsStarterCityCenter(bool isStarter){bIsStarterCityCenter = isStarter;}
};
