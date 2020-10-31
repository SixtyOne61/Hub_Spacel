// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "Mesh/LocationInformation.h"
#include "SpacelProceduralMeshComponent.generated.h"

static int32 DebugDrawProceduralMeshCollision = 0;

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
	void generateMesh(FName _profileName, int const& _nbPoint);
	
	// -- get / set
	inline void setCubeSize(FVector const& _cubeSize) { CubeSize = _cubeSize; }
	inline void setEdges(TArray<FLocationInformation> && _edges) { this->EdgesPosition = std::move(_edges); }
	inline void setOwnerLocation(FVector const& _ownerLocation) { m_ownerLocation = _ownerLocation; }

protected:
	void addTriangles(TArray<int32> & _out, int _deb) const;

	/* call when something hit this procedural mesh */
	UFUNCTION()
	void onHit(class UPrimitiveComponent* _comp, class AActor* _otherActor, class UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit);

	/* manage hit */
	bool hit(FVector const& _impactPoint);

protected:
	/* size of cube */
	UPROPERTY(VisibleAnywhere)
	FVector CubeSize = FVector::ZeroVector;

    /* half of cubeSize */
    FVector m_halfCubeSize = FVector::ZeroVector;

	/* location of owner */
	FVector m_ownerLocation = FVector::ZeroVector;

	/* list of all edges */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FLocationInformation> EdgesPosition;

	/* list of overlap object */
	TArray<AActor*> m_overlapActors;

    /* profile name for collision */
    FName m_collisionProfileName = "";

    /* number of point in mesh */
    int m_nbPoint = 0;
};