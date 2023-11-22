// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Valoria/ValoriaCam.h"
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
	void DrawMarquee();
	void UpdateSelectedCharacters(AValoriaCam* valoriaCam, AValoriaCharacter* selectedValoria);
	void SelectCharactersUnderDrawnRectangle(AValoriaCam* valoriaCam);
	void HandleMarqueeSelection();
	virtual void DrawHUD() override;
	void MarqueePressed();
	void MarqueeReleased();
	void MarqueeHeld();


	// public variables
	bool bCanDrawSelection =false;

private:

	// Private Variables
	FVector2D startMousePos;
	FVector2D currentMousePos;
	bool bIsDrawing{false};
	int8 marqueeMoveAllowTimerCounter = 0;
	bool bRunTimer = false;
	TArray<AActor*> SelectedActors;
};
