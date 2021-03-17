// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Enum/SpacelEnum.h"
#include "EffectDataAsset.generated.h"

USTRUCT()
struct HUB_SPACEL_API FEffect
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	EEffect Effect {};

	UPROPERTY(EditAnywhere)
	class UTexture2D* Icone { nullptr };

	UPROPERTY(EditAnywhere)
	FLinearColor BackgroundColor { };
};

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UEffectDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	FEffect getEffect(EEffect _type) const;

public:
	UPROPERTY(EditAnywhere)
	TArray<FEffect> Effects;	
};
