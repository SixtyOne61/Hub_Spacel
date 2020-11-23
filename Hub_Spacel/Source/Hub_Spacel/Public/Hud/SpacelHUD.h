// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SpacelHUD.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API ASpacelHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	ASpacelHUD();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TSubclassOf<class UUserWidget> GameWidgetClass { nullptr };

	UPROPERTY()
	TSubclassOf<UUserWidget> PreparePhaseWidgetClass { nullptr };
};
