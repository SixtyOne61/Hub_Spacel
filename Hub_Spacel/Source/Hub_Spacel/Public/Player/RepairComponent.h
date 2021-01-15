// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/PlayerActorComponent.h"
#include "RepairComponent.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API URepairComponent : public UPlayerActorComponent
{
	GENERATED_BODY()

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

	/* network */
	UFUNCTION()
	void OnRep_Matiere();

	/* call from client */
	UFUNCTION()
	void OnToggleRepair(bool _on);

	UFUNCTION(Reliable, Server)
	void RPCServerOnToggleRepair(bool _on);

	/* call from server */
	UFUNCTION()
	void OnRepairProtection(bool _on);

	UFUNCTION()
	void OnRepairSupport(bool _on);

	/* clean or start timer */
	void onRepair(bool _on, FTimerHandle& _handle, void(URepairComponent::* _callback)());

	/* repair protection */
	UFUNCTION()
	void RepairProtection();

	/* repair support */
	UFUNCTION()
	void RepairSupport();

	void repair(TArray<FVector> & _removedLocations, TArray<FVector> & _locations);

private:
	UPROPERTY(ReplicatedUsing = "OnRep_Matiere")
	float RU_Matiere = 0.0f;

	UPROPERTY()
	FTimerHandle RepairProtectionHandle {};

	UPROPERTY()
	FTimerHandle RepairSupportHandle {};
};
