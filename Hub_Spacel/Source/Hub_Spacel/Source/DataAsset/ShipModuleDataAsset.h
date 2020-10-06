// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ShipModuleDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UShipModuleDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
    class UProceduralModuleDataAsset const* GetModule(uint8 const& _flag);

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<class UProceduralModuleDataAsset*> Modules;
};
