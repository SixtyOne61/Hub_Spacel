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
	void SetTitle(FString const& _title);

	UFUNCTION(BlueprintImplementableEvent)
	void SetDesc(FString const& _title);
	
	UFUNCTION(BlueprintImplementableEvent)
	void ShowDesc(bool _visible);

public:
	EMission Type {};
};
