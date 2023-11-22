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
	CityCenter
};
UENUM(BlueprintType)
enum class EBuildingOwner:uint8
{
	self,
	ownerless,
	ally,
	enemy,
	neutral
};
class AValoriaCam;
class USceneComponent;
class UWidgetComponent;
class AValoriaCharacter;
class UStaticMesh;
class ABuildingBanner;
class UBoxComponent;
class AMapBorder;
class AValoriaWorker;
class AValoriaAI;
class UParticleSystem;
class USoundCue;
UCLASS()
class VALORIA_API ABuilding : public AActor
{
	GENERATED_BODY()

public:

	// public functions

	ABuilding();
	virtual void Tick(float DeltaTime) override;
	void DamageBuilding(float damage);

	// BlueprintImplementableEvent Functions
	UFUNCTION(BlueprintImplementableEvent)
	void BP_ConstructionHUD(bool active,int constructNum,ABuilding* building);
	
	// public variables
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details)
	USceneComponent* flagStarterPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details)
	USceneComponent* characterStarterPoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Sounds, meta = (AllowPrivateAccess = "true"))
	USoundCue* destroyBaseSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Sounds, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* destroyBaseParticle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details)
	int32 workerNumber{0};

	UPROPERTY()
	TArray<AValoriaCharacter*>buidlingWorkers;

	UPROPERTY()
	ABuildingBanner* buildingBannerRelated;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UMaterial* buildingMat;

	UPROPERTY()
	AValoriaAI* valoriaAIRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Details, meta = (AllowPrivateAccess = "true"))
	FString capitalName ;
	
	int32 buildingMaxWorker{3};
	int32 buildingWorkPointsIndex;
	FVector bannerLocation;
	bool bConstructionProgressStarted{false};
	bool bConstructionIsBuilt{false};
	bool bBuildingHasBanner{false};

private:


	// private variables


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* Widget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* box;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UStaticMesh* level1Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UStaticMesh* level2Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UStaticMesh* level3Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UMaterial* buildingGreenMat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UMaterial* buildingRedMat;


	bool bUpdatingNeeds{true};



	// Subclasses


	// Timer handles

	// Private Functions






protected:


	
	// Protocted Functions

	virtual void BeginPlay() override;
	void PlacingBuilding();
	void MovingBuildingToFindRightPlace();
	void UpdateBuildingNeeds();
	void ConstructionProcess(float DeltaTime);
	void ValidateBuildLocation(FVector loc);
	void CheckCanBuild();
	void LineTraceFloorCheckers();

	// protected variables

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* BuildingMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float constrcutionFinishValue = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float constructionCounter{0};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Details, meta = (AllowPrivateAccess = "true"))
	AMapBorder* BorderRef;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details)
	float workersStartWorkDistance = {400.f};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details)
	float buildingRadius = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details)
	float buildingHeight = 300.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Details)
	bool bCanCheck{true};

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Details)
	bool bBuildingIsAllowedToBeBuilt{false};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details)
	int32 wood;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details)
	int32 stone;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details)
	int32 gold;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details)
	int32 science;

	UPROPERTY()
	AValoriaCam* valoriaCam;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Details)
	EBuildingOwner buildingOwner;

	int32 buildingLevel {0};
	EBuildingType buildingType;
	FHitResult Hit;
	bool bEdge1;
	bool bEdge2;
	bool bEdge3;
	bool bEdge4;
	bool bIsBuildingSpawned{false};
	bool bBuildingPlaced{false};
	bool bIsBuildingSelected{false};
	float constructionProgressSpeed{200.f};
	

public:

	// Getter
	FORCEINLINE bool GetBuildingIsAllowedToBeBuilt()const {return bBuildingIsAllowedToBeBuilt;}
	FORCEINLINE bool GetConstructionIsBuilt()const {return bConstructionIsBuilt;}
	FORCEINLINE float GetWorkersStartWorkDistance()const {return workersStartWorkDistance;}
	FORCEINLINE float GetconstructionCounter()const {return constructionCounter;}
	FORCEINLINE int32 GetStone()const {return stone;}
	FORCEINLINE int32 GetWood()const {return wood;}
	FORCEINLINE int32 GetGold()const {return gold;}
	FORCEINLINE int32 GetScience()const {return science;}
	FORCEINLINE EBuildingType GetBuildingType()const {return buildingType;}
	FORCEINLINE UStaticMeshComponent* GetBuildingMesh()const {return BuildingMesh;}
	FORCEINLINE bool GetIsBuildingSpawned()const {return bIsBuildingSpawned;}
	FORCEINLINE bool GetIsBuildingSelected()const {return bIsBuildingSelected;}
	FORCEINLINE UBoxComponent* GetBox()const {return box;}
	FORCEINLINE UStaticMesh* GetLevel1Mesh() const {return level1Mesh;}


	// Setter
	FORCEINLINE void SetIsBuildingSpawned(bool IsSpawned){bIsBuildingSpawned = IsSpawned;}
	FORCEINLINE void SetBuildingIsAllowedToBeBuilt(bool canPlace){bBuildingIsAllowedToBeBuilt = canPlace;}
	FORCEINLINE void SetIsBuildingSelected(bool isSelected){bIsBuildingSelected = isSelected;}
	FORCEINLINE void SetBuildingOwner(EBuildingOwner owner){buildingOwner = owner;}
};
