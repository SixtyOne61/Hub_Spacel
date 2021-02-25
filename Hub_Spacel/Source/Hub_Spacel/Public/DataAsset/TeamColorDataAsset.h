// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Styling/SlateColor.h"
#include "TeamColorDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UTeamColorDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UFUNCTION()
	FSlateColor GetColor(FString const& _team) const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateColor Team1 {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateColor Team2 {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateColor Team3 {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateColor Team4 {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateColor Team5 {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSlateColor Team6 {};
	
};
