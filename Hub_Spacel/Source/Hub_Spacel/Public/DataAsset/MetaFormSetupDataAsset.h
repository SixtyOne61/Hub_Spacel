// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MetaFormSetupDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UMetaFormSetupDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	FString DefaultPath {};

	UPROPERTY(EditAnywhere)
	class UStaticMesh* AttackStaticMesh{ nullptr };

	UPROPERTY(EditAnywhere)
	class UStaticMesh* ProtectionStaticMesh{ nullptr };

	UPROPERTY(EditAnywhere)
	class UStaticMesh* SupportStaticMesh{ nullptr };
};
