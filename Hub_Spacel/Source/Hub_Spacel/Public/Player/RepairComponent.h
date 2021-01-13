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

	UFUNCTION()
	void OnHitProtection();

	UFUNCTION()
	void OnHitSupport();

	UFUNCTION()
	void OnRep_Matiere();

private:
	UPROPERTY(ReplicatedUsing = "OnRep_Matiere")
	float RU_Matiere = 0.0f;
};
