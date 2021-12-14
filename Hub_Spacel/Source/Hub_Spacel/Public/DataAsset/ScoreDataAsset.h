// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Styling/SlateColor.h"
#include "Enum/SpacelEnum.h"
#include "ScoreDataAsset.generated.h"

USTRUCT(BlueprintType)
struct HUB_SPACEL_API FScoreData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EScoreType Type {};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FSlateColor SlateColor {};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class UTexture2D* Logo{};
};

/**
 * 
 */
UCLASS(BlueprintType)
class HUB_SPACEL_API UScoreDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void FillInfo(EScoreType _type, FScoreData & _out) const;

public:
	UPROPERTY(EditAnywhere)
	TArray<FScoreData> Scores {};
};
