// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DitactitialDataAsset.generated.h"

USTRUCT(BlueprintType)
struct HUB_SPACEL_API FDitactitial
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BitmaskEnum = "EEffect"))
	FVector2D Position {};

	UPROPERTY(EditAnywhere)
	FString Tips { };
};

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UDitactitialDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDitactitial> Tips {};
};
