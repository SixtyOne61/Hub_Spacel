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
	USpacelProceduralMeshComponent();

	UFUNCTION(BlueprintCallable, Category = "Generation")
	void generateMesh();
	
	// -- get / set
	inline void setCubeSize(float _cubeSize) { CubeSize = _cubeSize; }
	inline void setEdges(TArray<TSharedPtr<ChainedLocation>> && _edges) { m_edgesPosition = std::move(_edges); }
	inline void setOwnerLocation(FVector const& _ownerLocation) { m_ownerLocation = _ownerLocation; }

	// -- manage hit
	void hit(FVector const& _forward, FVector const& _impactPoint);

protected:
	void addTriangles(TArray<int32> & _out, int _deb) const;

protected:
	// size of cube
	UPROPERTY(VisibleAnywhere)
	float CubeSize;

	// location of owner
	FVector m_ownerLocation;

	// list of all edges
	TArray<TSharedPtr<ChainedLocation>> m_edgesPosition;
};
