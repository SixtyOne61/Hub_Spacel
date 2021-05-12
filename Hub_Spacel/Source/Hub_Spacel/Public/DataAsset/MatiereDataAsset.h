// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MatiereDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UMatiereDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NbMatterGiveByCube { 50 };
};
