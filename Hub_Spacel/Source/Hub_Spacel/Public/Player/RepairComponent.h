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

	/* trigger when player lost cube */
	UFUNCTION()
	void OnHitProtection();

	UFUNCTION()
	void OnHitSupport();

	/* call from server */
	UFUNCTION()
	void OnRepairProtection();

	UFUNCTION()
	void OnRepairSupport();

	/* clean or start timer */
	void onRepair(bool _on, FTimerHandle& _handle, void(URepairComponent::* _callback)());

	/* repair protection */
	UFUNCTION()
	void RepairProtection();

	/* repair support */
	UFUNCTION()
	void RepairSupport();

	void repair(TArray<FVector> & _removedLocations, TArray<FVector> & _locations, std::function<void(void)> _onRep, FTimerHandle& _handle);

	/* call when ship is kill */
	void kill();

private:
	UPROPERTY()
	FTimerHandle RepairProtectionHandle {};

	UPROPERTY()
	FTimerHandle RepairSupportHandle {};

	bool m_isRepairProtection { false };
	bool m_isRepairSupport { false };
};
