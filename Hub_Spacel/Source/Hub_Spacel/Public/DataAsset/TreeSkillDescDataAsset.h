// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Enum/SpacelEnum.h"
#include "TreeSkillDescDataAsset.generated.h"

USTRUCT()
struct HUB_SPACEL_API FDesc
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	ESkillType Type {};

	UPROPERTY(EditAnywhere)
	TArray<FString> DescByLevel {};
};

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UTreeSkillDescDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	FDesc const& getDesc(ESkillType _type) const;

public:
	UPROPERTY(EditAnywhere)
	TArray<FDesc> Descs;
};
