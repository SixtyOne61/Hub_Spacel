// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RankingUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API URankingUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "Setup")
	void SetTeam(FString const& _name, FSlateColor const& _color, int32 _score);
};
