// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "MeshDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UMeshDataAsset : public UDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	int32 EnvironmentVoxelSize { };

	UPROPERTY(EditAnywhere)
	class UMaterialInstance* MaterialEnvironmentVoxel = nullptr;
};
