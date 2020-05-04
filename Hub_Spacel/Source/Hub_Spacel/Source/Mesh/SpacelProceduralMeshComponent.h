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

    virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Generation")
	void generateMesh(FName _profileName);
	
	// -- get / set
	inline void setCubeSize(FVector const& _cubeSize) { CubeSize = _cubeSize; }
	inline void setEdges(TArray<TSharedPtr<ChainedLocation>> && _edges) { m_edgesPosition = std::move(_edges); }
    inline TArray<TSharedPtr<ChainedLocation>> const& getEdges() const { return m_edgesPosition; }
	inline void setOwnerLocation(FVector const& _ownerLocation) { m_ownerLocation = _ownerLocation; }

protected:
	void addTriangles(TArray<int32> & _out, int _deb) const;

	// -- call when something hit this procedural mesh
	UFUNCTION()
	void onHit(class UPrimitiveComponent* _comp, class AActor* _otherActor, class UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit);

	// -- manage hit
	bool hit(FVector const& _forward, FVector const& _impactPoint);

protected:
	// size of cube
	UPROPERTY(VisibleAnywhere)
	FVector CubeSize;

	// location of owner
	FVector m_ownerLocation;

	// list of all edges
	TArray<TSharedPtr<ChainedLocation>> m_edgesPosition;

	// list of overlap object
	TArray<AActor*> m_overlapActors;

    // profile name for collision
    FName m_collisionProfileName;
};
