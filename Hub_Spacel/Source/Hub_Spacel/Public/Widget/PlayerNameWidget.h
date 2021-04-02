// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerNameWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UPlayerNameWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Setup")
	void SetPlayerName(FString const& _name, FSlateColor const& _TeamColor);
};
