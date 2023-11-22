// Fill out your copyright notice in the Description page of Project Settings.


#include "MapBorder.h"
#include "Components/BoxComponent.h"
AMapBorder::AMapBorder()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	borderbox = CreateDefaultSubobject<UBoxComponent>(TEXT("border box"));
	borderbox->SetupAttachment(GetRootComponent());
	cityCenterboxChecker = CreateDefaultSubobject<UBoxComponent>(TEXT("city Center box Checker"));
	cityCenterboxChecker->SetupAttachment(GetRootComponent());
	borderStatus = EBorderStatus::ownerless;
}

void AMapBorder::BeginPlay()
{
	Super::BeginPlay();
}

void AMapBorder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AMapBorder::UpdateBorderOwnerFromBlueprint()
{
	UpdateBorderOwner(borderStatus);
}


