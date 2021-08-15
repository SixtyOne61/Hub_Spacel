// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CleanFieldWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UCleanFieldWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void NativeConstruct() override;

protected:
	UFUNCTION()
	void OnKill(int32 _killer, int32 _killed);	
};
