// Fill out your copyright notice in the Description page of Project Settings.

#include "SpacelProceduralMeshComponent.h"
#include <algorithm>

USpacelProceduralMeshComponent::USpacelProceduralMeshComponent()
	: CubeSize(0.0f)
	, m_ownerLocation(FVector::ZeroVector)
{

}

void USpacelProceduralMeshComponent::generateMesh()
{
	TArray<FVector> vertices;
	TArray<int32> triangles;
	TArray<FVector> normals;

	for (TSharedPtr<ChainedLocation> const& point : m_edgesPosition)
	{
		EFace mask = point->getMask();
		FVector center = point->getCenter();

		// TO DO : only display if we havent all neighboor ?

		// read mask from linkPos
		if (!EnumHasAllFlags(mask, EFace::Top))
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(0, CubeSize, 0));
			vertices.Add(center + FVector(CubeSize, CubeSize, 0));
			vertices.Add(center + FVector(CubeSize, CubeSize, CubeSize));
			vertices.Add(center + FVector(0, CubeSize, CubeSize));

			normals.Add(FVector(0, 1, 0));
			normals.Add(FVector(0, 1, 0));
			normals.Add(FVector(0, 1, 0));
			normals.Add(FVector(0, 1, 0));
			addTriangles(triangles, deb);
		}
		if (!EnumHasAllFlags(mask, EFace::Bot))
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(CubeSize, 0, CubeSize));
			vertices.Add(center + FVector(CubeSize, 0, 0));
			vertices.Add(center + FVector(0, 0, 0));
			vertices.Add(center + FVector(0, 0, CubeSize));

			normals.Add(FVector(0, -1, 0));
			normals.Add(FVector(0, -1, 0));
			normals.Add(FVector(0, -1, 0));
			normals.Add(FVector(0, -1, 0));
			addTriangles(triangles, deb);
		}
		if (!EnumHasAllFlags(mask, EFace::Right))
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(CubeSize, 0, CubeSize));
			vertices.Add(center + FVector(CubeSize, CubeSize, CubeSize));
			vertices.Add(center + FVector(CubeSize, CubeSize, 0));
			vertices.Add(center + FVector(CubeSize, 0, 0));

			normals.Add(FVector(0, 0, 1));
			normals.Add(FVector(0, 0, 1));
			normals.Add(FVector(0, 0, 1));
			normals.Add(FVector(0, 0, 1));
			addTriangles(triangles, deb);
		}
		if (!EnumHasAllFlags(mask, EFace::Left))
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(0, 0, CubeSize));
			vertices.Add(center + FVector(0, 0, 0));
			vertices.Add(center + FVector(0, CubeSize, 0));
			vertices.Add(center + FVector(0, CubeSize, CubeSize));

			normals.Add(FVector(0, 0, -1));
			normals.Add(FVector(0, 0, -1));
			normals.Add(FVector(0, 0, -1));
			normals.Add(FVector(0, 0, -1));
			addTriangles(triangles, deb);
		}
		if (!EnumHasAllFlags(mask, EFace::Front))
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(0, CubeSize, 0));
			vertices.Add(center + FVector(0, 0, 0));
			vertices.Add(center + FVector(CubeSize, 0, 0));
			vertices.Add(center + FVector(CubeSize, CubeSize, 0));

			normals.Add(FVector(1, 0, 0));
			normals.Add(FVector(1, 0, 0));
			normals.Add(FVector(1, 0, 0));
			normals.Add(FVector(1, 0, 0));
			addTriangles(triangles, deb);
		}
		if (!EnumHasAllFlags(mask, EFace::Back))
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(CubeSize, 0, CubeSize));
			vertices.Add(center + FVector(0, 0, CubeSize));
			vertices.Add(center + FVector(0, CubeSize, CubeSize));
			vertices.Add(center + FVector(CubeSize, CubeSize, CubeSize));

			normals.Add(FVector(-1, 0, 0));
			normals.Add(FVector(-1, 0, 0));
			normals.Add(FVector(-1, 0, 0));
			normals.Add(FVector(-1, 0, 0));
			addTriangles(triangles, deb);
		}
	}

	TArray<FVector2D> UV0;
	UV0.Add(FVector2D(1, 1));
	UV0.Add(FVector2D(0, 1));
	UV0.Add(FVector2D(1, 0));
	UV0.Add(FVector2D(0, 0));
	TArray<FProcMeshTangent> tangents;
	TArray<FLinearColor> vertexColors;

	// setup collision
	SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	SetCollisionObjectType(ECollisionChannel::ECC_PhysicsBody);
	bUseComplexAsSimpleCollision = false;

	CreateMeshSection_LinearColor(0, vertices, triangles, normals, UV0, vertexColors, tangents, true);

	// Enable collision data
	AddCollisionConvexMesh(vertices);
}

void USpacelProceduralMeshComponent::hit(FVector const& _impactPoint)
{
	int size = m_edgesPosition.Num();
	m_edgesPosition.RemoveAll([&](TSharedPtr<ChainedLocation> _point)
	{
		return FVector::Distance(_point->getCenter() + m_ownerLocation, _impactPoint) <= CubeSize;
	});

	if (size != m_edgesPosition.Num())
	{
		generateMesh();
	}
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