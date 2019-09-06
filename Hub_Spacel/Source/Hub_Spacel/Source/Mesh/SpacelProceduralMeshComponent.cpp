// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacelProceduralMeshComponent.h"

void USpacelProceduralMeshComponent::generateMesh()
{
	TArray<FVector> vertices;
	TArray<int32> triangles;

	for (TSharedPtr<ChainedLocation> const& point : m_edgesPosition)
	{
		EFace mask = point->getMask();
		FVector center = point->getCenter();

		// TO DO : only display if we havent all neighboor ?

		// read mask from linkPos
		if (!EnumHasAllFlags(mask, EFace::Top))
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(0, m_cubeSize, 0));
			vertices.Add(center + FVector(m_cubeSize, m_cubeSize, 0));
			vertices.Add(center + FVector(m_cubeSize, m_cubeSize, m_cubeSize));
			vertices.Add(center + FVector(0, m_cubeSize, m_cubeSize));
			addTriangles(triangles, deb);
		}
		if (!EnumHasAllFlags(mask, EFace::Bot))
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(m_cubeSize, 0, m_cubeSize));
			vertices.Add(center + FVector(m_cubeSize, 0, 0));
			vertices.Add(center + FVector(0, 0, 0));
			vertices.Add(center + FVector(0, 0, m_cubeSize));
			addTriangles(triangles, deb);
		}
		if (!EnumHasAllFlags(mask, EFace::Right))
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(m_cubeSize, 0, m_cubeSize));
			vertices.Add(center + FVector(m_cubeSize, m_cubeSize, m_cubeSize));
			vertices.Add(center + FVector(m_cubeSize, m_cubeSize, 0));
			vertices.Add(center + FVector(m_cubeSize, 0, 0));
			addTriangles(triangles, deb);
		}
		if (!EnumHasAllFlags(mask, EFace::Left))
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(0, 0, m_cubeSize));
			vertices.Add(center + FVector(0, 0, 0));
			vertices.Add(center + FVector(0, m_cubeSize, 0));
			vertices.Add(center + FVector(0, m_cubeSize, m_cubeSize));
			addTriangles(triangles, deb);
		}
		if (!EnumHasAllFlags(mask, EFace::Front))
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(0, m_cubeSize, 0));
			vertices.Add(center + FVector(0, 0, 0));
			vertices.Add(center + FVector(m_cubeSize, 0, 0));
			vertices.Add(center + FVector(m_cubeSize, m_cubeSize, 0));
			addTriangles(triangles, deb);
		}
		if (!EnumHasAllFlags(mask, EFace::Back))
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(m_cubeSize, 0, m_cubeSize));
			vertices.Add(center + FVector(0, 0, m_cubeSize));
			vertices.Add(center + FVector(0, m_cubeSize, m_cubeSize));
			vertices.Add(center + FVector(m_cubeSize, m_cubeSize, m_cubeSize));
			addTriangles(triangles, deb);
		}
	}

	TArray<FVector> normals;
	TArray<FVector2D> UV0;
	TArray<FProcMeshTangent> tangents;
	TArray<FLinearColor> vertexColors;

	CreateMeshSection_LinearColor(0, vertices, triangles, normals, UV0, vertexColors, tangents, true);

	// Enable collision data
	ContainsPhysicsTriMeshData(true);
}

void USpacelProceduralMeshComponent::addTriangles(TArray<int32> & _out, int _deb) const
{
	int triangles[] = {
		_deb, _deb + 2, _deb + 1,
		_deb, _deb + 3, _deb + 2
	};

	for(int numEdge : triangles)
	{
		_out.Add(numEdge);
	}
}