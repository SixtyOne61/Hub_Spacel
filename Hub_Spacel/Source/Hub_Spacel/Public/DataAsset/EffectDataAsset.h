// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Enum/SpacelEnum.h"
#include "EffectDataAsset.generated.h"

USTRUCT(BlueprintType)
struct HUB_SPACEL_API FEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EEffect Type {};

	UPROPERTY(EditAnywhere)
	EEffectLocation TypeLocation {};

	UPROPERTY(EditAnywhere)
	class UTexture2D* Icone { nullptr };

	UPROPERTY(EditAnywhere)
	FLinearColor BackgroundColor { };

	UPROPERTY(EditAnywhere)
	FString Title {};
};

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UEffectDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	FEffect const* getEffect(EEffect _type) const;

public:
	UPROPERTY(EditAnywhere)
	TArray<FEffect> Effects;	
};
