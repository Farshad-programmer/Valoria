// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Valoria/ValoriaCharacter.h"
#include "ValoriaWorker.generated.h"

/**
 * 
 */

class ABarracks;
class AValoriaAI;
UCLASS()
class VALORIA_API AValoriaWorker : public AValoriaCharacter
{
	GENERATED_BODY()


public:
	AValoriaWorker();
	virtual void Tick(float DeltaSeconds) override;

	void StartBuilding();
	void StartWork();
	void CheckCharacterDistanceWithBuilding();


	// AI functions
	void AIMoveToBuildingLocation(AValoriaAI* AIRef);
	
private:






	// AI private functions

protected:
	virtual void BeginPlay() override;
public:


};




