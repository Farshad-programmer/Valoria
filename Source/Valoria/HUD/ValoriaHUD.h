// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ValoriaHUD.generated.h"

/**
 * 
 */
UCLASS()
class VALORIA_API AValoriaHUD : public AHUD
{
	GENERATED_BODY()


public:
	virtual void Tick(float DeltaSeconds) override;
	void HandleMarqueeSelection();
	virtual void DrawHUD() override;

	void MarqueePressed();
	void MarqueeReleased();
	void MarqueeHeld();

	bool bCanDrawSelection =false;

private:
	FVector2D startMousePos;
	FVector2D currentMousePos;
	bool bIsDrawing{false};
	int8 marqueeMoveAllowTimerCounter = 0;
	bool bRunTimer = false;

	TArray<AActor*> SelectedActors;
};
