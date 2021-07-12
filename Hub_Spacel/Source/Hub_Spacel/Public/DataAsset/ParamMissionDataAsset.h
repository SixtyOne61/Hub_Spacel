// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ParamMissionDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UParamMissionDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
};

UCLASS()
class HUB_SPACEL_API UParamPirateDataAsset : public UParamMissionDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class AActor> PirateClass;
};

UCLASS()
class HUB_SPACEL_API UParamCometDataAsset : public UParamMissionDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class AComet> CometClass{ nullptr };
};

UCLASS()
class HUB_SPACEL_API UParamGoldDataAsset : public UParamMissionDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class AGold> GoldClass{ nullptr };
};
