// Fill out your copyright notice in the Description page of Project Settings.


#include "House.h"

AHouse::AHouse()
{
	workersStartWorkDistance = 400.f;
	wood = 20;
	stone = 10;
	gold = 2;
	science = 0;

	buildingType = EBuildingType::house;
}
