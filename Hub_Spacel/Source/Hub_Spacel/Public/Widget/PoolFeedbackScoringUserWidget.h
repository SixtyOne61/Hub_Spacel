// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/SpacelEnum.h"
#include "PoolFeedbackScoringUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UPoolFeedbackScoringUserWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	void NativeConstruct() override;

private:
	UFUNCTION()
	void OnScored(EScoreType _type, int32 _value);

	UFUNCTION()
	void OnAnimationEnd(UPointUserWidget* _widget);

protected:
	UPROPERTY()
	TArray<class UPointUserWidget*> AvailableWidget;
};
