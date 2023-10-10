// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Building.generated.h"


UCLASS()
class VALORIA_API ABuilding : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Mesh, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* BuildingMesh;



public:	
	ABuilding();

protected:
	virtual void BeginPlay() override;
	int32 buildingLevel {0};
public:	
	virtual void Tick(float DeltaTime) override;

};
