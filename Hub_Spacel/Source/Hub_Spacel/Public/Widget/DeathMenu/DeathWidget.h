// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/SpacelEnum.h"
#include "DeathWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UDeathWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	void NativeConstruct() override;
	void NativeTick(const FGeometry& _myGeometry, float _deltaSeconde) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Setup")
	void BP_UpdateTimer(int _seconde);

	UFUNCTION(BlueprintImplementableEvent, Category = "Setup")
	void BP_EnableRespawn();

	UFUNCTION(BlueprintCallable, Category = "Action")
	void Respawn();

	UFUNCTION(BlueprintImplementableEvent, Category = "Event")
	void BP_OnAddEffect(EEffect _effect);

private:
	/* update timer before enable respawn */
	void updateTimer(float _deltaSeconde);

protected:
	/* timer before enable respawn */
	float m_currentTimer { 0.0f };
};
