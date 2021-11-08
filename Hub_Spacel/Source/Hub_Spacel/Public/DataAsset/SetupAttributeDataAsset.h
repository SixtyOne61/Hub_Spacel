// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StaticMeshDataAsset.h"
#include "SetupAttributeDataAsset.generated.h"

/**
 * TO DO deprecated remove
 */
UCLASS()
class HUB_SPACEL_API USetupAttributeDataAsset : public UStaticMeshDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	FString DefaultPath {};

	UPROPERTY(EditAnywhere)
	FString HeavyPath {};
};
