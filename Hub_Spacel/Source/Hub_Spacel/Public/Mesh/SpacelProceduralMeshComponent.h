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
    virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Generation")
	void generateMesh(FName _profileName, int _nbPoint, TArray<FLocationInformation> & _locations);

protected:
	void addTriangles(TArray<int32> & _out, int _deb) const;

	/* call when something hit this procedural mesh */
	UFUNCTION()
	void onHit(class UPrimitiveComponent* _comp, class AActor* _otherActor, class UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit);

	/* manage hit */
	bool hit(FVector const& _impactPoint);

public:
	/* size of cube */
	UPROPERTY(VisibleAnywhere)
	FVector CubeSize { };

	UPROPERTY(VisibleAnywhere)
	FVector OwnerLocation { };
};
