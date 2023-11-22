// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapBorder.generated.h"

UENUM(BlueprintType)
enum class EBorderStatus:uint8
{
	self,
	ownerless,
	ally,
	neutral,
	enemy
};

class UBoxComponent;
UCLASS()
class VALORIA_API AMapBorder : public AActor
{
	GENERATED_BODY()
	
public:	
	AMapBorder();
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Details)
	EBorderStatus borderStatus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Details,meta = (AllowPrivateAccess = "true"))
	bool bBorderHasCityCenter{false};

	UFUNCTION(BlueprintCallable)
	void UpdateBorderOwnerFromBlueprint();

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateBorderOwner(EBorderStatus status);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* borderbox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Details, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* cityCenterboxChecker;

	
public:	


};
