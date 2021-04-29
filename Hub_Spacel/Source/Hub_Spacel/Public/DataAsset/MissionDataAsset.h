// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Enum/SpacelEnum.h"
#include "MissionDataAsset.generated.h"

USTRUCT(BlueprintType)
struct HUB_SPACEL_API FMission
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMission Type {};

	UPROPERTY(EditAnywhere)
	FString MissionTitle { };

	UPROPERTY(EditAnywhere)
	int32 RewardValue { 0 };
	
	UPROPERTY(EditAnywhere)
	int32 ConditionValue { 0 };

	UPROPERTY(EditAnywhere)
	FString Team {};
};

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UMissionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	inline FMission getMission(EMission _type) const
	{
		for (auto const& mission : Missions)
		{
			if (mission.Type == _type)
			{
				return mission;
			}
		}

		ensure(false);
		return {};
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMission> Missions {};
};
