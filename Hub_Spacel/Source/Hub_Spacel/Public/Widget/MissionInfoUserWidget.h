// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/SpacelEnum.h"
#include "MissionInfoUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UMissionInfoUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetTitle(FString const& _title);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetDesc(FString const& _title);
	
	UFUNCTION(BlueprintImplementableEvent)
	void BP_ShowDesc(bool _visible);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetTimer(FString const& _titleWithTimer);

	void startTimer(int8 _duration);

	UFUNCTION()
	void UpdateTimer();

public:
	EMission Type {};
	FString Title {};
	int8 Duration {};

private:
	int8 m_currentTimer {};
};
