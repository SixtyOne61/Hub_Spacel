// Fill out your copyright notice in the Description page of Project Settings.

#include "SpacelProceduralMeshComponent.h"
#include "DrawDebugHelpers.h"
#include <algorithm>

USpacelProceduralMeshComponent::USpacelProceduralMeshComponent()
	: UProceduralMeshComponent(FObjectInitializer())
{
}

void USpacelProceduralMeshComponent::BeginPlay()
{
    Super::BeginPlay();
    OnComponentHit.AddDynamic(this, &USpacelProceduralMeshComponent::onHit);
}

void USpacelProceduralMeshComponent::generateMesh(FName _collisionProfileName)
{
    this->m_collisionProfileName = _collisionProfileName;

    // clear all
    ClearAllMeshSections();
    ClearCollisionConvexMeshes();

    int nbEdge = this->m_edgesPosition.Num();

    TArray<int32> triangles;
    triangles.Reserve(nbEdge * 6);

    int maxSize = nbEdge * 24;
	TArray<FVector> vertices;
    vertices.Reserve(maxSize);
    TArray<FVector> normals;
    normals.Reserve(maxSize);
    TArray<FLinearColor> vertexColors;
    vertexColors.Reserve(maxSize);
    TArray<FVector2D> UV0;
    UV0.Reserve(maxSize);

	FVector half = this->CubeSize / 2.0f;

	for (TSharedPtr<ChainedLocation> point : this->m_edgesPosition)
	{
		point->createBox(this->m_ownerLocation);

		EFace mask = point->getMask();
		FVector center = point->getCenter();

		// TO DO : only display if we havent all neighboor ?

		// read mask from linkPos
		if (!EnumHasAllFlags(mask, EFace::Top))
		{
            int deb = vertices.Num();
            vertices.Add(center + FVector(-half.X, half.Y, -half.Z));
            vertices.Add(center + FVector(half.X, half.Y, -half.Z));
            vertices.Add(center + FVector(half.X, half.Y, half.Z));
            vertices.Add(center + FVector(-half.X, half.Y, half.Z));

            UV0.Add(FVector2D(0, 0));
            UV0.Add(FVector2D(1, 0));
            UV0.Add(FVector2D(1, 1));
            UV0.Add(FVector2D(0, 1));

            vertexColors.Add(FLinearColor(1.f, 0.f, 0.f));
            vertexColors.Add(FLinearColor(0.f, 0.f, 1.f));
            vertexColors.Add(FLinearColor(1.f, 0.f, 0.f));
            vertexColors.Add(FLinearColor(0.f, 1.f, 0.f));

			normals.Add(FVector(0, 1, 0));
			normals.Add(FVector(0, 1, 0));
			normals.Add(FVector(0, 1, 0));
			normals.Add(FVector(0, 1, 0));
			addTriangles(triangles, deb);
		}
		if (!EnumHasAllFlags(mask, EFace::Bot))
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(half.X, -half.Y, half.Z));
			vertices.Add(center + FVector(half.X, -half.Y, -half.Z));
			vertices.Add(center + FVector(-half.X, -half.Y, -half.Z));
            vertices.Add(center + FVector(-half.X, -half.Y, half.Z));

            UV0.Add(FVector2D(1, 1));
            UV0.Add(FVector2D(1, 0));
            UV0.Add(FVector2D(0, 0));
            UV0.Add(FVector2D(0, 1));

            vertexColors.Add(FLinearColor(0.f, 0.f, 1.f));
            vertexColors.Add(FLinearColor(1.f, 0.f, 0.f));
            vertexColors.Add(FLinearColor(1.f, 0.f, 0.f));
            vertexColors.Add(FLinearColor(0.f, 1.f, 0.f));

			normals.Add(FVector(0, -1, 0));
			normals.Add(FVector(0, -1, 0));
			normals.Add(FVector(0, -1, 0));
			normals.Add(FVector(0, -1, 0));
			addTriangles(triangles, deb);
		}
		if (!EnumHasAllFlags(mask, EFace::Right))
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(half.X, -half.Y, half.Z));
			vertices.Add(center + FVector(half.X, half.Y, half.Z));
			vertices.Add(center + FVector(half.X, half.Y, -half.Z));
            vertices.Add(center + FVector(half.X, -half.Y, -half.Z));

            UV0.Add(FVector2D(0, 1));
            UV0.Add(FVector2D(1, 1));
            UV0.Add(FVector2D(1, 0));
            UV0.Add(FVector2D(0, 0));

            vertexColors.Add(FLinearColor(0.f, 0.f, 1.f));
            vertexColors.Add(FLinearColor(1.f, 0.f, 0.f));
            vertexColors.Add(FLinearColor(1.f, 0.f, 0.f));
            vertexColors.Add(FLinearColor(0.f, 1.f, 0.f));

			normals.Add(FVector(0, 0, 1));
			normals.Add(FVector(0, 0, 1));
			normals.Add(FVector(0, 0, 1));
			normals.Add(FVector(0, 0, 1));
			addTriangles(triangles, deb);
		}
		if (!EnumHasAllFlags(mask, EFace::Left))
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(-half.X, -half.Y, half.Z));
			vertices.Add(center + FVector(-half.X, -half.Y, -half.Z));
			vertices.Add(center + FVector(-half.X, half.Y, -half.Z));
            vertices.Add(center + FVector(-half.X, half.Y, half.Z));

            UV0.Add(FVector2D(0, 1));
            UV0.Add(FVector2D(0, 0));
            UV0.Add(FVector2D(1, 0));
            UV0.Add(FVector2D(0, 0));

            vertexColors.Add(FLinearColor(0.f, 0.f, 1.f));
            vertexColors.Add(FLinearColor(1.f, 0.f, 0.f));
            vertexColors.Add(FLinearColor(1.f, 0.f, 0.f));
            vertexColors.Add(FLinearColor(0.f, 1.f, 0.f));

			normals.Add(FVector(0, 0, -1));
			normals.Add(FVector(0, 0, -1));
			normals.Add(FVector(0, 0, -1));
			normals.Add(FVector(0, 0, -1));
			addTriangles(triangles, deb);
		}
		if (!EnumHasAllFlags(mask, EFace::Front))
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(-half.X, half.Y, -half.Z));
			vertices.Add(center + FVector(-half.X, -half.Y, -half.Z));
			vertices.Add(center + FVector(half.X, -half.Y, -half.Z));
            vertices.Add(center + FVector(half.X, half.Y, -half.Z));

            UV0.Add(FVector2D(0, 1));
            UV0.Add(FVector2D(0, 0));
            UV0.Add(FVector2D(1, 0));
            UV0.Add(FVector2D(1, 1));

            vertexColors.Add(FLinearColor(0.f, 0.f, 1.f));
            vertexColors.Add(FLinearColor(1.f, 0.f, 0.f));
            vertexColors.Add(FLinearColor(1.f, 0.f, 0.f));
            vertexColors.Add(FLinearColor(0.f, 1.f, 0.f));

			normals.Add(FVector(1, 0, 0));
			normals.Add(FVector(1, 0, 0));
			normals.Add(FVector(1, 0, 0));
			normals.Add(FVector(1, 0, 0));
			addTriangles(triangles, deb);
		}
		if (!EnumHasAllFlags(mask, EFace::Back))
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(half.X, -half.Y, half.Z));
			vertices.Add(center + FVector(-half.X, -half.Y, half.Z));
			vertices.Add(center + FVector(-half.X, half.Y, half.Z));
            vertices.Add(center + FVector(half.X, half.Y, half.Z));

            UV0.Add(FVector2D(1, 0));
            UV0.Add(FVector2D(0, 0));
            UV0.Add(FVector2D(0, 1));
            UV0.Add(FVector2D(1, 1));

            vertexColors.Add(FLinearColor(0.f, 0.f, 1.f));
            vertexColors.Add(FLinearColor(1.f, 0.f, 0.f));
            vertexColors.Add(FLinearColor(1.f, 0.f, 0.f));
            vertexColors.Add(FLinearColor(0.f, 1.f, 0.f));

			normals.Add(FVector(-1, 0, 0));
			normals.Add(FVector(-1, 0, 0));
			normals.Add(FVector(-1, 0, 0));
			normals.Add(FVector(-1, 0, 0));
			addTriangles(triangles, deb);
		}
	}

	// setup collision
	SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    SetCollisionProfileName(this->m_collisionProfileName);
	bUseComplexAsSimpleCollision = true;
	SetNotifyRigidBodyCollision(true);

	CreateMeshSection_LinearColor(0, vertices, triangles, normals, UV0, vertexColors, TArray<FProcMeshTangent>(), true);

	// Enable collision data
	AddCollisionConvexMesh(vertices);
}

bool USpacelProceduralMeshComponent::hit(FVector const& _forward, FVector const& _impactPoint)
{
	FVector endPoint = _impactPoint + _forward * CubeSize;

	if(m_edgesPosition.RemoveAll([&](TSharedPtr<ChainedLocation> _point)
	{
		if (_point->hasAllMask())
		{
			return false;
		}		

		FVector hitLocation;
		FVector hitNormal;
		float hitTime;
		return FMath::LineExtentBoxIntersection(_point->getBox(), _impactPoint, endPoint, FVector::ZeroVector, hitLocation, hitNormal, hitTime);
	}) != 0)
	{
        // TO DO; only update needed part
		generateMesh(this->m_collisionProfileName);
		return true;
	}

	return false;
}

void USpacelProceduralMeshComponent::addTriangles(TArray<int32> & _out, int _deb) const
{
	int triangles[] = {
		_deb, _deb + 1, _deb + 2,
		_deb, _deb + 2, _deb + 3
	};

	for(int numEdge : triangles)
	{
		_out.Add(numEdge);
	}
}

void USpacelProceduralMeshComponent::onHit(class UPrimitiveComponent* _comp, class AActor* _otherActor, class UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
    if (!_otherActor)
    {
        return;
    }

    DrawDebugSphere(GetWorld(), _otherActor->GetActorLocation(), 200.0f, 12, FColor::Blue, false, 30.0f, 128, 10.0f);
    DrawDebugSphere(GetWorld(), _hit.ImpactPoint, 250.0f, 12, FColor::Red, false, 30.0f, 128, 10.0f);

	// check if it's a bullet type
    if (hit(_hit.ImpactNormal, _hit.ImpactPoint))
    {
        _otherActor->Destroy();
    }
}