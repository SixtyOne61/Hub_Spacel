// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MeshFactory.generated.h"

UCLASS()
class HUB_SPACEL_API AMeshFactory : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMeshFactory();

	UFUNCTION(BlueprintCallable, Category = "Generation")
	void GenerateVoxel();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
	class UMeshDataAsset* MeshDataAsset { nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Procedural Mesh")
	class USpacelProceduralMeshComponent* Voxel { nullptr };
};
