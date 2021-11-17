// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EndScreenWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UEndScreenWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Setup")
	void BP_Winner(bool _isWin);

protected:
	void NativeConstruct() override;

	void setup();
};
