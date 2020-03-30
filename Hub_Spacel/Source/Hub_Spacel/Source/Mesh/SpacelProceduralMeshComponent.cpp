// Fill out your copyright notice in the Description page of Project Settings.

#include "SpacelProceduralMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Hub_Spacel/Source/Projectile/SimpleBullet.h"
#include <algorithm>

USpacelProceduralMeshComponent::USpacelProceduralMeshComponent()
	: CubeSize(0.0f)
	, m_ownerLocation(FVector::ZeroVector)
{
	OnComponentHit.AddDynamic(this, &USpacelProceduralMeshComponent::onHit);
}

void USpacelProceduralMeshComponent::generateMesh()
{
    // clear all
    ClearAllMeshSections();
    ClearCollisionConvexMeshes();

	TArray<FVector> vertices;
	TArray<int32> triangles;
    TArray<FVector> normals;
    TArray<FLinearColor> vertexColors;
    TArray<FVector2D> UV0;

	float half = CubeSize / 2.0f;

	for (TSharedPtr<ChainedLocation> point : m_edgesPosition)
	{
		point->createBox(m_ownerLocation);

		EFace mask = point->getMask();
		FVector center = point->getCenter();

		// TO DO : only display if we havent all neighboor ?

		// read mask from linkPos
		if (!EnumHasAllFlags(mask, EFace::Top))
		{
            int deb = vertices.Num();
            vertices.Add(center + FVector(-half, half, -half));
            vertices.Add(center + FVector(half, half, -half));
            vertices.Add(center + FVector(half, half, half));
            vertices.Add(center + FVector(-half, half, half));

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
			vertices.Add(center + FVector(half, -half, half));
			vertices.Add(center + FVector(half, -half, -half));
			vertices.Add(center + FVector(-half, -half, -half));
            vertices.Add(center + FVector(-half, -half, half));

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
			vertices.Add(center + FVector(half, -half, half));
			vertices.Add(center + FVector(half, half, half));
			vertices.Add(center + FVector(half, half, -half));
            vertices.Add(center + FVector(half, -half, -half));

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
			vertices.Add(center + FVector(-half, -half, half));
			vertices.Add(center + FVector(-half, -half, -half));
			vertices.Add(center + FVector(-half, half, -half));
            vertices.Add(center + FVector(-half, half, half));

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
			vertices.Add(center + FVector(-half, half, -half));
			vertices.Add(center + FVector(-half, -half, -half));
			vertices.Add(center + FVector(half, -half, -half));
            vertices.Add(center + FVector(half, half, -half));

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
			vertices.Add(center + FVector(half, -half, half));
			vertices.Add(center + FVector(-half, -half, half));
			vertices.Add(center + FVector(-half, half, half));
            vertices.Add(center + FVector(half, half, half));

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

	TArray<FProcMeshTangent> tangents;

	// setup collision
	SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	bUseComplexAsSimpleCollision = true;
	SetNotifyRigidBodyCollision(true);

	CreateMeshSection_LinearColor(0, vertices, triangles, normals, UV0, vertexColors, TArray<FProcMeshTangent>() /*tangents*/, true);

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
		generateMesh();
		return true;
	}

	return false;
}

void USpacelProceduralMeshComponent::addTriangles(TArray<int32> & _out, int _deb) const
{
	int triangles[] = {
		_deb, _deb + 1, _deb + 2,
		_deb, _deb + 2, _deb + 3
        //_deb + 2, _deb + 3, _deb
	};

	for(int numEdge : triangles)
	{
		_out.Add(numEdge);
	}
}

void USpacelProceduralMeshComponent::onHit(class UPrimitiveComponent* _comp, class AActor* _otherActor, class UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
	// check if it's a bullet type
	ASimpleBullet* pBullet = Cast<ASimpleBullet>(_otherActor);
	if (pBullet)
	{
		// find where and destroy the right edge
		if (hit(pBullet->getLaunchForward(), _hit.ImpactPoint))
		{
			
		}

        // all case, it's better to delete bullet

        // destroy bullet
        pBullet->Destroy();
	}
}