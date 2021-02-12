// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "InputDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UInputDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeUp {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeDown {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TimeReset {};
};
