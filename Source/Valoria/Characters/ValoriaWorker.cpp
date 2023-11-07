// Fill out your copyright notice in the Description page of Project Settings.


#include "ValoriaWorker.h"

#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Valoria/ValoriaCam.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NiagaraComponent.h"
#include "Valoria/Buildings/Building.h"
#include "NavigationSystem.h"
#include "Valoria/Resources/ResourceMaster.h"
#include "Kismet/KismetMathLibrary.h"
#include "Valoria/AI/AValoriaAI.h"
#include "Valoria/Buildings/Barracks.h"

AValoriaWorker::AValoriaWorker()
{

}

void AValoriaWorker::BeginPlay()
{
	Super::BeginPlay();

}


void AValoriaWorker::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bCanCheckForStartWork)
	{
		CheckCharacterDistanceWithBuilding();
	}
}



void AValoriaWorker::CheckCharacterDistanceWithBuilding()
{
	if (buildingRef)
	{
		float distance = buildingRef->GetDistanceTo(this);
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::FromInt(distance));
		if (distance <= buildingRef->GetWorkersStartWorkDistance())
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.02f, FColor::Red, TEXT("distance <= buildingRef->GetWorkersStartWorkDistance()"));
			StartBuilding();
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.02f, FColor::Red, TEXT("distance  > GetWorkersStartWorkDistance"));
		}
	}
	if (resourceRef)
	{
		float distance = resourceRef->GetDistanceTo(this);
		//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::FromInt(distance));
		if (distance <= resourceRef->GetWorkersStartWorkDistance())
		{
			StartWork();
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.02f, FColor::Orange, TEXT("distance  > GetWorkersStartWorkDistance"));
		}
	}

}

void AValoriaWorker::AIMoveToBuildingLocation(AValoriaAI* AIRef)
{
	AAIController* DefaultAIController = Cast<AAIController>(GetController());
	if (DefaultAIController && buildingRef)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.02f, FColor::Green, TEXT("AIMoveToBuildingLocation"));
		buildingRef->valoriaAIRef = AIRef;
		DefaultAIController->MoveToLocation(buildingRef->GetActorLocation());
		if(buildingRef->valoriaAIRef->enemyStatus == EAIStatus::ally)
		{
			buildingRef->SetBuildingOwner(EBuildingOwner::ally);
		}
		else if(buildingRef->valoriaAIRef->enemyStatus == EAIStatus::enemy)
		{
			buildingRef->SetBuildingOwner(EBuildingOwner::enemy);
		}
	}
}


void AValoriaWorker::StartBuilding()
{
	GetCharacterMovement()->StopMovementImmediately();
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("start Building!!!"));
	float distance = buildingRef->GetDistanceTo(this);
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	if (buildingRef && buildingRef->buildingWorkPointsIndex < buildingRef->buildingMaxWorker && distance <= buildingRef->GetWorkersStartWorkDistance())
	{
		if (animInstance && BuildingAnimation)
		{
			animInstance->Montage_Play(BuildingAnimation, 1.f);
			buildingRef->bConstructionProgressStarted = true;
			bIsStartedWork = true;
			buildingRef->buildingWorkPointsIndex++;
			//GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, FString::FromInt(buildingRef->buildingWorkPointsIndex));
			buildingRef->workerNumber++;
			buildingRef->buidlingWorkers.Add(this);
			bCanCheckForStartWork = false;
			if (GetMesh())
			{
				GetMesh()->SetRenderCustomDepth(true);
			}
		}

	}
	AValoriaCam* valoriaCam = Cast<AValoriaCam>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (valoriaCam)
	{
		if (valoriaCam->IsAllNewWorkersStartedWork(valoriaCam->players))
		{
			valoriaCam->DeselectAllCharacters();
		}
	}
}



void AValoriaWorker::StartWork()
{
	GetCharacterMovement()->StopMovementImmediately();
	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("start to work"));

	float distance = resourceRef->GetDistanceTo(this);
	UAnimInstance* animInstance = GetMesh()->GetAnimInstance();
	//GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor::Yellow, FString::FromInt(resourceRef->buildingWorkPoints.Num()));
	if (resourceRef && resourceRef->buildingWorkPointsIndex < resourceRef->buildingWorkPoints.Num() && distance <= resourceRef->GetWorkersStartWorkDistance())
	{
		if (animInstance && BuildingAnimation)
		{
			animInstance->Montage_Play(BuildingAnimation, 1.f);
			resourceRef->bWorkProgressStarted = true;
			bIsStartedWork = true;
			resourceRef->buildingWorkPointsIndex++;
			resourceRef->workerNumber++;
			resourceRef->buidlingWorkers.Add(this);
			bCanCheckForStartWork = false;
			if (GetMesh())
			{
				GetMesh()->SetRenderCustomDepth(true);
			}
		}

	}
	AValoriaCam* valoriaCam = Cast<AValoriaCam>(UGameplayStatics::GetPlayerPawn(this, 0));
	if (valoriaCam)
	{
		if (valoriaCam->IsAllNewWorkersStartedWork(valoriaCam->players))
		{
			valoriaCam->DeselectAllCharacters();
		}
	}
}




