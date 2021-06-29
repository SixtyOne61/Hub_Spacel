// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FlyingGameModeDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UFlyingGameModeDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	int RemainingGameTime { 690 }; // 11'30

	UPROPERTY(EditAnywhere)
	int RemainingChooseModuleTime { 20 };

	UPROPERTY(EditAnywhere)
	int EndModuleTime { 5 };

	UPROPERTY(EditAnywhere)
	int RemainingLeaveTime { 20 };
};
