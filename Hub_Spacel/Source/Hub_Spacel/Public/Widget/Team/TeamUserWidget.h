// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TeamUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UTeamUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(Category = "Setup", EditAnywhere, BlueprintReadWrite)
	FString Team {};
};
