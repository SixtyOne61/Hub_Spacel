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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (BitmaskEnum = "EEffect"))
	EMission Type {};

	UPROPERTY(EditAnywhere)
	FString MissionTitle { };
};

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UMissionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	inline void fillMission(EMission _type, FMission & _mission) const
	{
		for (auto const& mission : Missions)
		{
			if (mission.Type == _type)
			{
				_mission = mission;
				return;
			}
		}

		ensure(false);
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FMission> Missions {};
};
