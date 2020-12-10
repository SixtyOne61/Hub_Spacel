// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LaserDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API ULaserDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	FName CollisionProfileName { "PlayerProjectile" };

	UPROPERTY(EditAnywhere)
	class UStaticMesh* LaserMesh { nullptr };
};
