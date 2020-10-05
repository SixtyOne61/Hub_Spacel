// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ProceduralModuleDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UProceduralModuleDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (Bitmask, BitmaskEnum = "EShipModuleType", UseEnumValuesAsMaskValuesInEditor = "true"))
    uint8 ModuleType = 0x0;
};
