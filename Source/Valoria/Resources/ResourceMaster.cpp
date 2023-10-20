

#include "ResourceMaster.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Valoria/ValoriaCam.h"

AResourceMaster::AResourceMaster()
{
	PrimaryActorTick.bCanEverTick = true;
	ResourceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ResourceMesh"));
	ResourceMesh->SetupAttachment(RootComponent);
	Widget = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	Widget->SetupAttachment(ResourceMesh);
	WorkerPoint1 = CreateDefaultSubobject<USceneComponent>(TEXT("WorkerPoint1"));
	WorkerPoint1->SetupAttachment(ResourceMesh);
	WorkerPoint2 = CreateDefaultSubobject<USceneComponent>(TEXT("WorkerPoint2"));
	WorkerPoint2->SetupAttachment(ResourceMesh);
	WorkerPoint3 = CreateDefaultSubobject<USceneComponent>(TEXT("WorkerPoint3"));
	WorkerPoint3->SetupAttachment(ResourceMesh);
	if (WorkerPoint1 && WorkerPoint2 && WorkerPoint3)
	{
		WorkerPoint1->SetRelativeLocation(FVector(-310.f, 0.f, 0.f));
		WorkerPoint2->SetRelativeLocation(FVector(310.f, 0.f, 0.f));
		WorkerPoint3->SetRelativeLocation(FVector(0.f, -310.f, 0.f));
	}

	ResourceMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
}

void AResourceMaster::BeginPlay()
{
	Super::BeginPlay();
	valoriaCam = Cast<AValoriaCam>(UGameplayStatics::GetPlayerPawn(this, 0));
	buildingWorkPoints.Emplace(WorkerPoint1->GetRelativeLocation());
	buildingWorkPoints.Emplace(WorkerPoint2->GetRelativeLocation());
	buildingWorkPoints.Emplace(WorkerPoint3->GetRelativeLocation());

	valoriaCam = Cast<AValoriaCam>(UGameplayStatics::GetPlayerPawn(this, 0));
}

void AResourceMaster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bWorkProgressStarted)
	{
		resourceCounter += workProgressSpeed * workerNumber * DeltaTime;
		resourcePlusCounter += workProgressSpeed * resourseIncomePower * workerNumber * DeltaTime;
		
		GEngine->AddOnScreenDebugMessage(-1, 0.01f, FColor::Yellow, FString::FromInt(resourcePlusCounter));
		if (valoriaCam && resourcePlusCounter >= resourceIncomeDelay)
		{
			resourcePlusCounter = 0;
			if (resourceType == EResourceType::wood)
			{
				valoriaCam->UpdateWood(true,resourceIncomeValue);
			}
			if (resourceType == EResourceType::stone)
			{
				valoriaCam->UpdateStone(true,resourceIncomeValue);
			}
			if (resourceType == EResourceType::gold)
			{
				valoriaCam->UpdateGold(true,resourceIncomeValue);
			}
			if (resourceType == EResourceType::science)
			{
				valoriaCam->UpdateScience(true,resourceIncomeValue);
			}
		}
	}

}

