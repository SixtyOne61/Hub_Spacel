// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Enum/SpacelEnum.h"
#include "GameStateDataAsset.generated.h"

USTRUCT(BlueprintType)
struct HUB_SPACEL_API FScoreByType
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BitmaskEnum = "EEffect"))
	EScoreType Type {};

	UPROPERTY(EditAnywhere)
	int32 Value { };
};

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UGameStateDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	int32 getScore(EScoreType _type) const
	{
		for (auto const& score : Scores)
		{
			if (score.Type == _type)
			{
				return score.Value;
			}
		}

		ensure(false);
		return 0;
	}
public:
	UPROPERTY(EditAnywhere)
	TArray<FScoreByType> Scores;

	UPROPERTY(EditAnywhere)
	int32 ThresholdForUltimate { 2000 };
};
