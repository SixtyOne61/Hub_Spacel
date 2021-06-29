// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "HomingMissileDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UHomingMissileDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeBeforeLock { 0.2f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedPreLock { 2000.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpeedAfterLock{ 3000.0f };
};
