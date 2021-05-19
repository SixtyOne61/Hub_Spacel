// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/SpacelEnum.h"
#include "PreparePhaseWidget.generated.h"

/**
 * 
 */
 // DEPRECATED
UCLASS()
class HUB_SPACEL_API UPreparePhaseWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetupOutline(FSlateColor _color);

	UFUNCTION(BlueprintImplementableEvent)
	void OnLock();

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
	void LockPrepare();

	UFUNCTION()
	void StartGame();

	UFUNCTION()
	void SetupOwningTeam();

	UFUNCTION()
	void RegisterEvent();

	void updatePlayerInfo();

	UFUNCTION()
	void UpdateBonus(ESkillType _type, uint8 _level);

public:
	UPROPERTY(EditAnywhere)
	class UTeamColorDataAsset* Colors { nullptr };

	//UPROPERTY(EditAnywhere)
	//class UTreeSkillDescDataAsset* TreeSkillDesc { nullptr };

private:
	UPROPERTY()
	class UTextBlock* RemainingSkillPointTextBlock { nullptr };

	UPROPERTY()
	class UTextBlock* TimeTextBlock { nullptr };

	UPROPERTY()
	class UTextBlock* BonusTextBlock { nullptr };

	UPROPERTY()
	TArray<class USelectorSkillWidget*> SelectorSkillWidget { };

	UPROPERTY()
	TArray<class UPlayerCardWidget*> PlayerCardWidget { };

	UPROPERTY()
	FTimerHandle TimeHandle {};

	UPROPERTY()
	int RemainingTime { 60 };

	bool m_isLock { false };
};
