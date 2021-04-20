// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/PlayerActorComponent.h"
#include <functional>
#include "RepairComponent.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API URepairComponent : public UPlayerActorComponent
{
	GENERATED_BODY()

	friend class AShipPawn;

public:
	URepairComponent();

	void BeginPlay() override;
	
private:
	UFUNCTION()
	void OnUpdateMatiere(int _value);

	/* repair protection */
	bool onRepairProtection();

	/* repair support */
	bool onRepairSupport();

	bool repair(TArray<FVector> & _removedLocations, TArray<FVector> & _locations, std::function<void(void)> _onRep, int _minMatiere, int _effect);
};
