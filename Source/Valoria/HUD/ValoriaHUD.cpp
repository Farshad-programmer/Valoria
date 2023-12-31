// Fill out your copyright notice in the Description page of Project Settings.


#include "ValoriaHUD.h"

#include "Components/WidgetComponent.h"
#include "Valoria/ValoriaPlayerController.h"
#include "Engine/Canvas.h"
#include "Valoria/ValoriaCam.h"
#include "Kismet/GameplayStatics.h"
#include "Valoria/ValoriaCharacter.h"
#include "Valoria/Buildings/Building.h"
#include "Valoria/Resources/ResourceMaster.h"





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
	if (!bCanDrawSelection) return;
	DrawMarquee();
	APlayerController* playerController = GetOwningPlayerController();
	if (playerController)
	{
		TArray<AActor*>allSelectedActors;
		GetActorsInSelectionRectangle(AValoriaCharacter::StaticClass(), startMousePos, currentMousePos, allSelectedActors, false, false);
		AValoriaCam* valoriaCam = Cast<AValoriaCam>(playerController->GetPawn());
		for (auto act : allSelectedActors)
		{
			if (act->ActorHasTag("Player"))
			{
				if (bIsDrawing)
				{
					SelectedActors.AddUnique(act);
				}
			}
		}
		DeselectUnheldPlayers(allSelectedActors, valoriaCam);
		SelectCharactersUnderDrawnRectangle(valoriaCam);
	}
}
void AValoriaHUD::DrawMarquee()
{
	if (bIsDrawing)
	{
		float screenW = currentMousePos.X - startMousePos.X;
		float screenH = currentMousePos.Y - startMousePos.Y;
		FString TheFloatStr = FString::SanitizeFloat(startMousePos.X);
		DrawRect(FLinearColor(0.f, 0.11f, 0.92f, 0.2f), startMousePos.X, startMousePos.Y, screenW, screenH);
	}
}
void AValoriaHUD::DeselectUnheldPlayers(TArray<AActor*>chars, AValoriaCam* valoriaCamRef)
{
	int32 Index = 0;
	while (Index < SelectedActors.Num())
	{
		AActor* SelectedActor = SelectedActors[Index];
		if (SelectedActor)
		{
			int32 CharIndex = chars.Find(SelectedActor);
			if (CharIndex == -1)
			{
				AValoriaCharacter* ValoriaChar = Cast<AValoriaCharacter>(SelectedActor);
				if (ValoriaChar)
				{
					ValoriaChar->SetSelectionNiagaraVisibility(false);
					ValoriaChar->SetOverlayWidgetVisibility(false);
					ValoriaChar->SetIsSelected(false);
					valoriaCamRef->players.Remove(ValoriaChar);
					SelectedActors.RemoveAt(Index); 
					continue;
				}
			}
		}
		Index++;
	}

}
void AValoriaHUD::SelectCharactersUnderDrawnRectangle(AValoriaCam* valoriaCam)
{
	if ((currentMousePos - startMousePos).Size() > 100.f)
	{
		if (valoriaCam)
		{
			valoriaCam->SetIsMarqueeSelected(true);
			for (auto SelectedActor : SelectedActors)
			{
				AValoriaCharacter* selectedValoria = Cast<AValoriaCharacter>(SelectedActor);
				if (selectedValoria)
				{
					UpdateSelectedCharacters(valoriaCam, selectedValoria);
				}
			}
		}
	}
}
void AValoriaHUD::UpdateSelectedCharacters(AValoriaCam* valoriaCam, AValoriaCharacter* selectedValoria)
{
	if (selectedValoria->ActorHasTag("Player"))
	{
		selectedValoria->SetSelectionNiagaraVisibility(true);
		selectedValoria->SetOverlayWidgetVisibility(true);
		selectedValoria->SetCanRotateToEnemy(false);
		selectedValoria->SetCanCheckDistanceWithAI(false);
		selectedValoria->SetIsSelected(true);
		valoriaCam->players.AddUnique(selectedValoria);
		if (selectedValoria->GetIsStartedWork())
		{
			selectedValoria->StopWorkAnimation();
			if (selectedValoria->buildingRef && selectedValoria->GetIsStartedWork() && selectedValoria->GetMesh()->GetAnimInstance()->IsAnyMontagePlaying())
			{
				selectedValoria->buildingRef->buildingWorkPointsIndex--;
				selectedValoria->buildingRef->workerNumber--;
				selectedValoria->buildingRef->buidlingWorkers.Remove(selectedValoria);
				if (selectedValoria->buildingRef->buildingWorkPointsIndex < 0)
				{
					selectedValoria->buildingRef->buildingWorkPointsIndex = 0;
					selectedValoria->buildingRef->workerNumber = 0;
					selectedValoria->buildingRef->buidlingWorkers.Empty();
				}
				selectedValoria->buildingRef = nullptr;
				selectedValoria->SetOverlayWidgetVisibility(true);
			}

			if (selectedValoria->resourceRef && selectedValoria->GetIsStartedWork() && selectedValoria->GetMesh()->GetAnimInstance()->IsAnyMontagePlaying())
			{
				selectedValoria->resourceRef->buildingWorkPointsIndex--;
				selectedValoria->resourceRef->workerNumber--;
				selectedValoria->resourceRef->buidlingWorkers.Remove(selectedValoria);
				if (selectedValoria->resourceRef->buildingWorkPointsIndex < 0)
				{
					selectedValoria->resourceRef->buildingWorkPointsIndex = 0;
					selectedValoria->resourceRef->workerNumber = 0;
					selectedValoria->resourceRef->buidlingWorkers.Empty();
				}
				selectedValoria->resourceRef = nullptr;
			}
		}
		else
		{
			selectedValoria->SetCheckForStartWork(false);
			selectedValoria->SetIsStartedWork(false);
			selectedValoria->buildingRef = nullptr;
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
	SelectedActors.Empty();
	bIsDrawing = false;
	bCanDrawSelection = false;
	APlayerController* playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (playerController)
	{
		AValoriaCam* valoriaCam = Cast<AValoriaCam>(playerController->GetPawn());
		if (valoriaCam)
		{
			bRunTimer = true;
		}
	}
}
void AValoriaHUD::MarqueeHeld()
{
	if (bCanDrawSelection)
	{
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





//for (auto AllSelectedActor : allSelectedActors)
//	{
//		DrawDebugSphere(GetWorld(),AllSelectedActor->GetActorLocation(),200.f,25,FColor::Yellow);
//	}