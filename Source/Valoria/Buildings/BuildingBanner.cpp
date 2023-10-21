// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildingBanner.h"

ABuildingBanner::ABuildingBanner()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	BannerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Banner Mesh"));
	BannerMesh->SetupAttachment(RootComponent);
}

void ABuildingBanner::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABuildingBanner::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

