// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Building.generated.h"

UENUM(BlueprintType)
enum class EBuildingType:uint8
{
	house,
	Barracks,
};

class AValoriaCam;
class USceneComponent;
class UWidgetComponent;
class AValoriaCharacter;
class UStaticMesh;
class ABuildingBanner;
UCLASS()
class VALORIA_API ABuilding : public AActor
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* BuildingMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WorkerPoint1;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WorkerPoint2;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WorkerPoint3;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* Widget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UStaticMesh* level1Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UStaticMesh* level2Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UStaticMesh* level3Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UMaterial* buildingMat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UMaterial* buildingGreenMat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UMaterial* buildingRedMat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float constrcutionFinishValue = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float constructionCounter{0};

	bool bUpdatingNeeds{true};

public:	
	ABuilding();
	virtual void Tick(float DeltaTime) override;


	TArray<FVector>buildingWorkPoints;
	int32 buildingWorkPointsIndex;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details)
	int32 workerNumber{0};

	UPROPERTY()
	TArray<AValoriaCharacter*>buidlingWorkers;

	bool bConstructionProgressStarted{false};
	bool bConstructionIsBuilt{false};
	bool bBuildingHasBanner{false};

	UPROPERTY()
	ABuildingBanner* buildingBannerRelated;

	FVector bannerLocation;
protected:
	virtual void BeginPlay() override;
	void ValidateBuildLocation(FVector loc);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details)
	float workersStartWorkDistance = {400.f};
	int32 buildingLevel {0};
	FHitResult Hit;

	void CheckCanBuild();
	bool bCanCheck{true};

	UPROPERTY()
	AValoriaCam* valoriaCam;

	float constructionProgressSpeed{200.f};

	bool bIsBuildingSpawned{false};
	bool bBuildingPlaced{false};
	bool bBuildingIsAllowedToBeBuilt{false};

	// needs
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details)
	int32 wood;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details)
	int32 stone;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details)
	int32 gold;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details)
	int32 science;

	EBuildingType buildingType;
public:

	FORCEINLINE bool GetBuildingIsAllowedToBeBuilt()const {return bBuildingIsAllowedToBeBuilt;}
	FORCEINLINE bool GetConstructionIsBuilt()const {return bConstructionIsBuilt;}
	FORCEINLINE float GetWorkersStartWorkDistance()const {return workersStartWorkDistance;}
	FORCEINLINE int32 GetStone()const {return stone;}
	FORCEINLINE int32 GetWood()const {return wood;}
	FORCEINLINE int32 GetGold()const {return gold;}
	FORCEINLINE int32 GetScience()const {return science;}
	FORCEINLINE EBuildingType GetBuildingType()const {return buildingType;}
	FORCEINLINE UStaticMeshComponent* GetBuildingMesh()const {return BuildingMesh;}
	

	FORCEINLINE void SetIsBuildingSpawned(bool IsSpawned){bIsBuildingSpawned = IsSpawned;}
	FORCEINLINE void SetBuildingIsAllowedToBeBuilt(bool canPlace){bBuildingIsAllowedToBeBuilt = canPlace;}

};
