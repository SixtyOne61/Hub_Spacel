// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PirateDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UPirateDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	FString XmlPath {};
};
