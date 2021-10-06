// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SkillWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API USkillWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void BP_Setup(FSlateColor _color, class UTexture2D* _texture, class UTexture2D* _name);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetupMatere(int32 _value);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_Remove();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_Lock(bool _isUnlock);
};
