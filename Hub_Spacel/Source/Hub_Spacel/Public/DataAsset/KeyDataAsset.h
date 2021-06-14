// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "KeyDataAsset.generated.h"

USTRUCT(BlueprintType)
struct HUB_SPACEL_API FImageKey
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FKey Key { };

	UPROPERTY(EditAnywhere)
	class UTexture2D* Icone{ nullptr };
};

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UKeyDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	class UTexture2D* get(FKey const& _key)
	{
		for (FImageKey const& key : Keys)
		{
			if (key.Key == _key)
			{
				return key.Icone;
			}
		}

		return nullptr;
	}

	UPROPERTY(EditAnywhere)
	TArray<FImageKey> Keys;
};
