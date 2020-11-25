// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UWeaponDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	class UStaticMesh* WeaponMesh{ nullptr };
	
	UPROPERTY(EditAnywhere)
	FString DefaultWeaponPath {};

	UPROPERTY(EditAnywhere)
	FString HeavyWeaponPath {};
};
