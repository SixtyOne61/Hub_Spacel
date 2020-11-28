// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SupportDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API USupportDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	FString DefaultSupportPath {};

	UPROPERTY(EditAnywhere)
	class UStaticMesh* SupportMesh { nullptr };

	UPROPERTY(EditAnywhere)
	FString HeavySupportPath {};
};
