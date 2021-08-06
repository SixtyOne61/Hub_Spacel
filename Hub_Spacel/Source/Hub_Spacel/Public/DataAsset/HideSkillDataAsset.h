// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Enum/SpacelEnum.h"
#include "HideSkillDataAsset.generated.h"

USTRUCT(BlueprintType)
struct HUB_SPACEL_API FHideSkill
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EInput Type {};

	UPROPERTY(EditAnywhere)
	FKey Key { };

	UPROPERTY(EditAnywhere)
	FString Desc { };

	UPROPERTY(EditAnywhere)
	FString ActionName { };

	UPROPERTY(EditAnywhere)
	float Value { };
};

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UHideSkillDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	FHideSkill* get(EInput _input)
	{
		for (auto& hideSkill : HideSkills)
		{
			if (hideSkill.Type == _input)
			{
				return &hideSkill;
			}
		}

		return nullptr;
	}
	
public:
	UPROPERTY(EditAnywhere)
	TArray<FHideSkill> HideSkills {};
};
