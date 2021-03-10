// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Styling/SlateColor.h"
#include "TeamColorDataAsset.generated.h"

USTRUCT()
struct HUB_SPACEL_API FColorsType
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	FSlateColor SlateColor {};

	UPROPERTY(EditAnywhere)
	FColor Color {};

	template<typename T>
	T get() const;
};

template<typename T>
T FColorsType::get() const { ensure(false); return T(); }

template<>
inline FColor FColorsType::get() const { return Color; }

template<>
inline FSlateColor FColorsType::get() const { return SlateColor; }

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UTeamColorDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	template<typename T>
	T GetColor(FString const& _team) const;

public:
	UPROPERTY(EditAnywhere)
	TArray<FColorsType> TeamColors {};
};

template<typename T>
inline T UTeamColorDataAsset::GetColor(FString const& _team) const
{
	TArray<FString> names = { "Team 1", "Team 2", "Team 3", "Team 4", "Team 5", "Team 6" };
	for (int32 i = 0; i < names.Num(); ++i)
	{
		if (_team == names[i])
		{
			return TeamColors[i].get<T>();
		}
	}

	ensure(false);
	return T();
}
