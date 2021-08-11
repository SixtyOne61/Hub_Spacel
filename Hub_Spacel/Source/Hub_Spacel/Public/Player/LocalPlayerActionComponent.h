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
	~ULocalPlayerActionComponent();

	void BeginPlay() override;
	void TickComponent(float _deltaTime, ELevelTick _tickType, FActorComponentTickFunction* _thisTickFunction) override;

private:
	UFUNCTION()
	void OnUpdateTeam(FString const& _team);

	UFUNCTION()
	void AddEffect(EEffect _effect);

	UFUNCTION()
	void RemoveEffect(EEffect _effect);

private:
	class UMaterialInstanceDynamic* m_speedLineMaterial { nullptr };
};
