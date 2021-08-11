// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EndMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UEndMenuWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UTeamColorDataAsset* TeamColorDataAsset { nullptr };
};
