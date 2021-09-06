// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Enum/SpacelEnum.h"
#include "Curves/CurveFloat.h"
#include "CurveAnimationDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FVoxelAnimationCurve
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	EAnimationCurveType Type;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FRuntimeFloatCurve Curve;
};

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UCurveAnimationDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	inline FVoxelAnimationCurve* get(EAnimationCurveType _type)
	{
		for (FVoxelAnimationCurve & curve : Curves)
		{
			if (curve.Type == _type)
			{
				return &curve;
			}
		}

		return nullptr;
	}

public:
	UPROPERTY(Category = "Animation", BlueprintReadOnly, EditAnywhere)
	TArray<FVoxelAnimationCurve> Curves;
};
