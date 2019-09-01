// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "Hub_Spacel/Source/Engine/Location/ChainedLocation.h"
#include "SpacelProceduralMeshComponent.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API USpacelProceduralMeshComponent : public UProceduralMeshComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Generation")
	void generateMesh();
	
	// -- get / set
	inline void setCubeSize(float _cubeSize) { m_cubeSize = _cubeSize; }
	inline void setEdges(TArray<ChainedLocation> && _edges) { m_edgesPosition = std::move(_edges); }

protected:
	void addTriangles(TArray<int32> & _out, int _deb) const;

protected:
	// size of cube
	UPROPERTY(VisibleAnywhere)
	float m_cubeSize;

	// list of all edges
	TArray<ChainedLocation> m_edgesPosition;
};
