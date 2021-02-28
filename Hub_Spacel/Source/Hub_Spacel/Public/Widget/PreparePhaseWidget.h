// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/SpacelEnum.h"
#include "PreparePhaseWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UPreparePhaseWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetupOutline(FSlateColor _color);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void UpdateRemainingSkillPoint();

	UFUNCTION()
	void SetTime();

	UFUNCTION()
	void StartPrepare();

	UFUNCTION()
	void StartGame();

	UFUNCTION()
	void SetPlayerCard();

public:
	UPROPERTY(EditAnywhere)
	class UTeamColorDataAsset* Colors { nullptr };

private:
	UPROPERTY()
	class UTextBlock* RemainingSkillPointTextBlock { nullptr };

	UPROPERTY()
	class UTextBlock* TimeTextBlock { nullptr };

	UPROPERTY()
	class UPlayerCardWidget* Player1 { nullptr };

	UPROPERTY()
	class UPlayerCardWidget* Player2 { nullptr };

	UPROPERTY()
	FTimerHandle TimeHandle {};
	
	UPROPERTY()
	FTimerHandle SetPlayerCardHandle {};

	UPROPERTY()
	int RemainingTime { 60 };
};
