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
	void SetupOwningTeam();

	UFUNCTION()
	void OnClickLevel(ESkillType _type, uint8 _level);

	void updatePlayerInfo();

public:
	UPROPERTY(EditAnywhere)
	class UTeamColorDataAsset* Colors { nullptr };

private:
	UPROPERTY()
	class UTextBlock* RemainingSkillPointTextBlock { nullptr };

	UPROPERTY()
	class UTextBlock* TimeTextBlock { nullptr };

	UPROPERTY()
	TArray<class USelectorSkillWidget*> SelectorSkillWidget { };

	UPROPERTY()
	TArray<class UPlayerCardWidget*> PlayerCardWidget { };

	UPROPERTY()
	FTimerHandle TimeHandle {};

	UPROPERTY()
	int RemainingTime { 60 };
};
