// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ValoriaCam.generated.h"

/** Forward declaration to improve compiling times */
class UNiagaraSystem;
class AValoriaCharacter;
class AValoriaPlayerController;
class AValoriaHUD;
class ABuilding;
class ABuildingBanner;
class AValoriaInfantry;
UCLASS()
class VALORIA_API AValoriaCam : public APawn
{
	GENERATED_BODY()


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* ValeriaCamera;


public:
	// Sets default values for this pawn's properties
	AValoriaCam();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

		/** Time Threshold to know if it was a short press */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	float ShortPressThreshold;

	/** FX Class that we will spawn when clicking */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UNiagaraSystem* FXCursor;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;
	
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* SelectClickAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* Deselect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* SetDestinationClickAction;





	UPROPERTY()
	TArray<AValoriaCharacter*>players;

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Spawning)
	TSubclassOf<ABuilding> CityCenterToSpawn;
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Spawning)
	TSubclassOf<ABuilding> houseToSpawn;
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Spawning)
	TSubclassOf<ABuilding> BarracksToSpawn;
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Spawning)
	TSubclassOf<ABuildingBanner> buildingBannerToSpawn;
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Spawning)
	TSubclassOf<AValoriaInfantry> valoriaSwordSoldierToSpawn;
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Spawning)
	TSubclassOf<AValoriaInfantry> valoriaSpearmanToSpawn;
	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category= Spawning)
	TSubclassOf<AValoriaInfantry> valoriaCommanderToSpawn;

	void DeselectAllCharacters();
	bool IsAllNewWorkersStartedWork(TArray<AValoriaCharacter*> workers);
	void DestroyAllBanners();
	void DeselectAllBuildings();

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	ABuilding* buildingRef;

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly)
	ABuilding* resourceRef;

	UPROPERTY()
	AValoriaCharacter* PlayerTemp ;



	// blueprint callable functions

	UFUNCTION(BlueprintCallable)
	void SpawnConstruction(int32 constructionID);

	UFUNCTION(BlueprintCallable)
	void SpawnSoldier(int32 soldierCode,ABuilding* building);

	// blueprint implement event functions
	UFUNCTION(BlueprintImplementableEvent)
	void BP_ConstructionHUD(bool active,int constructNum,ABuilding* building);
	//constructNum => 0:building 1:Barracks

	void UpdateWood(bool plus,int32 amount);
	void UpdateGold(bool plus,int32 amount);
	void UpdateStone(bool plus,int32 amount);
	void UpdateScience(bool plus,int32 amount);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** Input handlers for SetDestination action. */
	void OnInputStarted();
	void OnSelectStarted();
	void OnSelectReleased();
	void OnDeselectStarted();
	void OnSetDestinationStarted();
	void OnSetDestinationReleased();


	




private:
	FVector CachedDestination;
	FHitResult Hit;
	float FollowTime; // For how long it has been pressed

	bool bIsPlayerSelected{false};
	bool bCanAdjustBuildingBannerPosition{false};

	UPROPERTY()
	ABuildingBanner* buildingBannerRef;
	TArray<ABuildingBanner*>AllBanners;

	UPROPERTY()
	TArray<AActor*> characters;

	bool bIsLeftMousePressed{false};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess = "true"))
	bool bMovingBanner{false};

	bool bAdjustingBanner{true};

	int adjustingBannerCounter = 0;
	bool bBuildingSelected{false};
	bool bRunCustomDepthSpecialMode{false};
	bool bMouseIsOnBanner{false};
	void RenderCustomDepthHandle();
	UPROPERTY()
	AValoriaPlayerController* valoriaPlayerController;

	UPROPERTY()
	APlayerController* playerController;

	bool bMarqueeSelected{false};
	bool bCanMarqueeMove{false};
	bool bCourserHitSuccessful;
	bool bCanPlaceBuilding{false};
	bool bIsPlacingBuidling {false};

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category= Game ,meta=(AllowPrivateAccess = "true"))
	int32 wood {100};

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category= Game ,meta=(AllowPrivateAccess = "true"))
	int32 stone {100};

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category= Game, meta=(AllowPrivateAccess = "true"))
	int32 gold {50};

	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category= Game, meta=(AllowPrivateAccess = "true"))
	int32 science {20};

public:	
		// All getter and setter here
	FORCEINLINE bool GetIsPlayerSelected()const {return bIsPlayerSelected;}
	FORCEINLINE bool GetCanPlaceBuilding()const {return bCanPlaceBuilding;}
	FORCEINLINE int32 GetWood()const {return wood;}
	FORCEINLINE int32 GetStone()const {return stone;}
	FORCEINLINE int32 GetGold()const {return gold;}
	FORCEINLINE int32 GetScience()const {return science;}
	FORCEINLINE bool GetIsPlacingBuilding()const {return bIsPlacingBuidling;}

	FORCEINLINE void SetIsPlayerSelected(bool isSelected){bIsPlayerSelected = isSelected;}
	FORCEINLINE void SetIsMarqueeSelected(bool isMarqueeSelected){bMarqueeSelected = isMarqueeSelected;}
	FORCEINLINE void SetCanMarqueeMove(bool canMarqueeMove){bCanMarqueeMove = canMarqueeMove;}
	FORCEINLINE void SetCanPlaceBuilding(bool canPlace){bCanPlaceBuilding = canPlace;}
	FORCEINLINE void SetIsPlacingBuidling(bool isPlacing){bIsPlacingBuidling = isPlacing;}

};
