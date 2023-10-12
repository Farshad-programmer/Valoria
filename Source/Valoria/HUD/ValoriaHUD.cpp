// Fill out your copyright notice in the Description page of Project Settings.


#include "ValoriaHUD.h"
#include "Valoria/ValoriaPlayerController.h"
#include "Engine/Canvas.h"
#include "Valoria/ValoriaCam.h"
#include "Kismet/GameplayStatics.h"
#include "Valoria/ValoriaCharacter.h"


void AValoriaHUD::DrawHUD()
{
	Super::DrawHUD();
	HandleMarqueeSelection();
}

void AValoriaHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bRunTimer)
	{
		marqueeMoveAllowTimerCounter++;

		if (marqueeMoveAllowTimerCounter > 10)
		{
			marqueeMoveAllowTimerCounter = 0;
			bRunTimer = false;
			APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
			AValoriaCam* valoriaCam = Cast<AValoriaCam>(PlayerController->GetPawn());
			if (valoriaCam)
			{
				valoriaCam->SetCanMarqueeMove(true);
			}
		}
	}
}

void AValoriaHUD::HandleMarqueeSelection()
{
	if (bIsDrawing)
	{
		float screenW = currentMousePos.X - startMousePos.X;
		float screenH = currentMousePos.Y - startMousePos.Y;
		FString TheFloatStr = FString::SanitizeFloat(startMousePos.X);
		DrawRect(FLinearColor(0.f, 0.11f, 0.92f, 0.2f), startMousePos.X, startMousePos.Y, screenW, screenH);
	}

	APlayerController* playerController = GetOwningPlayerController();
	if (playerController)
	{
		TArray<AActor*> SelectedActors;
		GetActorsInSelectionRectangle(AActor::StaticClass(), startMousePos, currentMousePos, SelectedActors, false, false);
		AValoriaCam* valoriaCam = Cast<AValoriaCam>(playerController->GetPawn());

		if ((currentMousePos - startMousePos).Size() > 100.f)
		{
			if (valoriaCam)
			{
				//valoriaCam->DeselectAllCharacters();
				valoriaCam->SetIsMarqueeSelected(true);
				for (auto SelectedActor : SelectedActors)
				{
					AValoriaCharacter* selectedValoria = Cast<AValoriaCharacter>(SelectedActor);
					if (selectedValoria)
					{
						if (selectedValoria->ActorHasTag("Player"))
						{
							selectedValoria->SetSelectionNiagaraVisibility(true);
							//selectedValoria->GetMesh()->SetRenderCustomDepth(true);
							valoriaCam->players.AddUnique(selectedValoria);
						}
					}
				}
			}
		}

	}
}

void AValoriaHUD::MarqueePressed()
{
	bIsDrawing = true;
	APlayerController* playerController = GetOwningPlayerController();
	if (playerController)
	{
		float locationX;
		float locationY;
		playerController->GetMousePosition(locationX, locationY);
		startMousePos.X = locationX;
		startMousePos.Y = locationY;
		currentMousePos = startMousePos;
	}
}

void AValoriaHUD::MarqueeReleased()
{
	bIsDrawing = false;
	APlayerController* playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (playerController)
	{
		AValoriaCam* valoriaCam = Cast<AValoriaCam>(playerController->GetPawn());
		if (valoriaCam)
		{
			bRunTimer = true;
			//valoriaCam->SetCanMarqueeMove(true);
		}
		FInputModeGameOnly InputMode;
		playerController->SetInputMode(InputMode);
	}
}

void AValoriaHUD::MarqueeHeld()
{
	if (bCanDrawSelection)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("MarqueeHeld "));

		APlayerController* playerController = GetOwningPlayerController();
		if (playerController)
		{
			float locationX;
			float locationY;
			playerController->GetMousePosition(locationX, locationY);
			currentMousePos.X = locationX;
			currentMousePos.Y = locationY;
		}
	}

}
