// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ProtectionDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UProtectionDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FString DefaultProtectionPath {};

	UPROPERTY(EditAnywhere)
	class UStaticMesh* DefaultStaticMesh { nullptr };
	
	UPROPERTY(EditAnywhere)
	FString HeavyProtectionPath {};

	UPROPERTY(EditAnywhere)
	class UStaticMesh* HeavyStaticMesh { nullptr };
};
