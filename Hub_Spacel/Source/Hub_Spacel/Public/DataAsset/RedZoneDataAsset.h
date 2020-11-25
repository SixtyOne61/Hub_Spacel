// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RedZoneDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API URedZoneDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	class UStaticMesh* RedZoneStaticMesh{ nullptr };
};
