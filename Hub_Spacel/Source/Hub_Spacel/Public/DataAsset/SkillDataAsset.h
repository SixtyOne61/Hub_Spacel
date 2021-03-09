// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Enum/SpacelEnum.h"
#include "SkillDataAsset.generated.h"

USTRUCT()
struct HUB_SPACEL_API FSkill
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	ESkill Skill {};

	UPROPERTY(EditAnywhere)
	uint32 CoundDown { 0 };

	UPROPERTY(EditAnywhere)
	uint32 Duration { 0 };
};

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API USkillDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	FSkill getSKill(ESkill _type) const;

public:
	UPROPERTY(EditAnywhere)
	TArray<FSkill> Skills;

	UPROPERTY(EditAnywhere)
	uint8 LevelSpecial { 2 };

	UPROPERTY(EditAnywhere)
	uint8 LevelMetaForm { 3 };
};
