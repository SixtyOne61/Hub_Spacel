// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/PlayerActorComponent.h"
#include "LocalPlayerActionComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HUB_SPACEL_API ULocalPlayerActionComponent : public UPlayerActorComponent
{
	GENERATED_BODY()
	
public:
	ULocalPlayerActionComponent();

	void BeginPlay() override;
	void TickComponent(float _deltaTime, ELevelTick _tickType, FActorComponentTickFunction* _thisTickFunction) override;

private:
	UFUNCTION()
	void OnUpdateTeam(FString const& _team);

private:
	class UMaterialInstanceDynamic* m_postProcessMaterial { nullptr };
};
