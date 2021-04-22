// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TutorialUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UTutorialUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void ShowDitactitial(FString const& _tips);
};
