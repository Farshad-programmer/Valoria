// Fill out your copyright notice in the Description page of Project Settings.


#include "CityCenter.h"

#include "Components/TextRenderComponent.h"
#include "Valoria/MapBorder/MapBorder.h"
#include "Components/TextRenderComponent.h"


ACityCenter::ACityCenter()
{
	capitalNameTextRender = CreateDefaultSubobject<UTextRenderComponent>(TEXT("capital Name Text Render"));
	capitalNameTextRender->SetupAttachment(GetRootComponent());

	workersStartWorkDistance = 400.f;
	wood = 30;
	stone = 30;
	gold = 20;
	science = 0;

	buildingType = EBuildingType::CityCenter;

}



void ACityCenter::BeginPlay()
{
	Super::BeginPlay();
	SetBuildingToStarterBuilding();
	
}


void ACityCenter::Tick(float DeltaTime)
{
	if (!bIsStarterCityCenter)
	{
		Super::Tick(DeltaTime);
		if (bBuildingPlaced)
		{
			if (BorderRef && !BorderRef->bBorderHasCityCenter)
			{
				BorderRef->bBorderHasCityCenter = true;
			}
		}
	}

}


void ACityCenter::SetBuildingToStarterBuilding()
{
	if (bIsStarterCityCenter && BuildingMesh && buildingMat)
	{
		constructionCounter = constrcutionFinishValue;
		BuildingMesh->SetMaterial(0,buildingMat);
	}
}
