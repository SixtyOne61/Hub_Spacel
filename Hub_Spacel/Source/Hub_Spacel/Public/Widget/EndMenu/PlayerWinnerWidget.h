// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerWinnerWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UPlayerWinnerWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetColor(FColor const& _color);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetStats(int _nbKill, int _score, int Precision);

};
