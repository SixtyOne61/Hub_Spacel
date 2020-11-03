// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WorldDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UWorldDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	uint8 NbChunckPerAxis { };

	UPROPERTY(EditAnywhere)
	uint8 NbCubePerChunckPerAxis { };

	UPROPERTY(EditAnywhere)
	int32 CubeSize { };
};
