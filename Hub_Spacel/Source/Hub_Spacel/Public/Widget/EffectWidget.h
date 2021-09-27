// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/SpacelEnum.h"
#include "EffectWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UEffectWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetEffect(FLinearColor _color, UTexture2D* _texture);

public:
	UPROPERTY()
	EEffect Effect;
};
