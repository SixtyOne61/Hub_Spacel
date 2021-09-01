// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CleanFieldTextWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UCleanFieldTextWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetText(FString const& _killer, FSlateColor const& _killerColor, FString const& _killed, FSlateColor const& _killedColor);
};
