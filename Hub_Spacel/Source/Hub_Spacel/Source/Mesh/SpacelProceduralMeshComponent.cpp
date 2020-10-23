// Fill out your copyright notice in the Description page of Project Settings.

#include "SpacelProceduralMeshComponent.h"
#include "DrawDebugHelpers.h"
#include <algorithm>

FAutoConsoleVariableRef CVARDebugDrawProceduralMeshCollision(
    TEXT("h.Draw.ProceduralMeshCollision"),
    DebugDrawProceduralMeshCollision,
    TEXT("Debug draw collision, 0 : disable, 1 : show impact point."),
    ECVF_Default
);

USpacelProceduralMeshComponent::USpacelProceduralMeshComponent()
	: UProceduralMeshComponent(FObjectInitializer())
{
}

void USpacelProceduralMeshComponent::BeginPlay()
{
    Super::BeginPlay();
    OnComponentHit.AddDynamic(this, &USpacelProceduralMeshComponent::onHit);
}

void USpacelProceduralMeshComponent::generateMesh(FName _collisionProfileName, int const& _nbPoint)
{
    m_collisionProfileName = _collisionProfileName;
    m_nbPoint = _nbPoint;

    // clear all
    ClearAllMeshSections();
    ClearCollisionConvexMeshes();

    int nbEdge = _nbPoint;

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

	m_halfCubeSize = this->CubeSize / 2.0f;

	for (FLocationInformation & point : this->EdgesPosition)
	{
        if (!point.Used)
        {
            continue;
        }

		FVector const& center = point.Location;

		// read mask from linkPos
		if (!EnumHasAllFlags(point.Mask, EFace::Top))
		{
            int deb = vertices.Num();
            vertices.Add(center + FVector(-m_halfCubeSize.X, m_halfCubeSize.Y, -m_halfCubeSize.Z));
            vertices.Add(center + FVector(m_halfCubeSize.X, m_halfCubeSize.Y, -m_halfCubeSize.Z));
            vertices.Add(center + FVector(m_halfCubeSize.X, m_halfCubeSize.Y, m_halfCubeSize.Z));
            vertices.Add(center + FVector(-m_halfCubeSize.X, m_halfCubeSize.Y, m_halfCubeSize.Z));

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
		if (!EnumHasAllFlags(point.Mask, EFace::Bot))
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(m_halfCubeSize.X, -m_halfCubeSize.Y, m_halfCubeSize.Z));
			vertices.Add(center + FVector(m_halfCubeSize.X, -m_halfCubeSize.Y, -m_halfCubeSize.Z));
			vertices.Add(center + FVector(-m_halfCubeSize.X, -m_halfCubeSize.Y, -m_halfCubeSize.Z));
            vertices.Add(center + FVector(-m_halfCubeSize.X, -m_halfCubeSize.Y, m_halfCubeSize.Z));

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
		if (!EnumHasAllFlags(point.Mask, EFace::Right))
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(m_halfCubeSize.X, -m_halfCubeSize.Y, m_halfCubeSize.Z));
			vertices.Add(center + FVector(m_halfCubeSize.X, m_halfCubeSize.Y, m_halfCubeSize.Z));
			vertices.Add(center + FVector(m_halfCubeSize.X, m_halfCubeSize.Y, -m_halfCubeSize.Z));
            vertices.Add(center + FVector(m_halfCubeSize.X, -m_halfCubeSize.Y, -m_halfCubeSize.Z));

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
		if (!EnumHasAllFlags(point.Mask, EFace::Left))
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(-m_halfCubeSize.X, -m_halfCubeSize.Y, m_halfCubeSize.Z));
			vertices.Add(center + FVector(-m_halfCubeSize.X, -m_halfCubeSize.Y, -m_halfCubeSize.Z));
			vertices.Add(center + FVector(-m_halfCubeSize.X, m_halfCubeSize.Y, -m_halfCubeSize.Z));
            vertices.Add(center + FVector(-m_halfCubeSize.X, m_halfCubeSize.Y, m_halfCubeSize.Z));

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
		if (!EnumHasAllFlags(point.Mask, EFace::Front))
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(-m_halfCubeSize.X, m_halfCubeSize.Y, -m_halfCubeSize.Z));
			vertices.Add(center + FVector(-m_halfCubeSize.X, -m_halfCubeSize.Y, -m_halfCubeSize.Z));
			vertices.Add(center + FVector(m_halfCubeSize.X, -m_halfCubeSize.Y, -m_halfCubeSize.Z));
            vertices.Add(center + FVector(m_halfCubeSize.X, m_halfCubeSize.Y, -m_halfCubeSize.Z));

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
		if (!EnumHasAllFlags(point.Mask, EFace::Back))
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(m_halfCubeSize.X, -m_halfCubeSize.Y, m_halfCubeSize.Z));
			vertices.Add(center + FVector(-m_halfCubeSize.X, -m_halfCubeSize.Y, m_halfCubeSize.Z));
			vertices.Add(center + FVector(-m_halfCubeSize.X, m_halfCubeSize.Y, m_halfCubeSize.Z));
            vertices.Add(center + FVector(m_halfCubeSize.X, m_halfCubeSize.Y, m_halfCubeSize.Z));

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
    SetCollisionProfileName(m_collisionProfileName);
	bUseComplexAsSimpleCollision = true;
	SetNotifyRigidBodyCollision(true);

	CreateMeshSection_LinearColor(0, vertices, triangles, normals, UV0, vertexColors, {}, true);

	// Enable collision data
	AddCollisionConvexMesh(vertices);
}

bool USpacelProceduralMeshComponent::hit(FVector const& _impactPoint)
{
    // for this moment we keep cubeSize instead half (more impact size)
    float radius = this->CubeSize.Size();
    FVector ownerLocation = m_ownerLocation;
    
    int nbPointDisabled = 0;

    // TO DO : add edge with hasAllMash, in an other array, and don't check it for remove ?
    for (auto & locationInformation : this->EdgesPosition)
    {
        if (locationInformation.Used)
        {
            // check dist to point
            if (FVector::Dist(locationInformation.Location + ownerLocation, _impactPoint) <= radius)
            {
                locationInformation.Used = false;
                ++nbPointDisabled;

                auto lb_updateNeighboor = [&](EFace const& _f1, EFace const& _f2)
                {
                    if (EnumHasAllFlags(locationInformation.Mask, _f1))
                    {
                        // safe check
                        if (int id = locationInformation.getNeighboorId(_f1); id != -1)
                        {
                            FLocationInformation & neigboor = this->EdgesPosition[id];
                            if (neigboor.Used)
                            {
                                neigboor.Mask ^= _f2;
                                neigboor.setNeighboorId(_f2);
                            }
                        }
                    }
                };

                lb_updateNeighboor(EFace::Top, EFace::Bot);
                lb_updateNeighboor(EFace::Bot, EFace::Top);
                lb_updateNeighboor(EFace::Front, EFace::Back);
                lb_updateNeighboor(EFace::Back, EFace::Front);
                lb_updateNeighboor(EFace::Right, EFace::Left);
                lb_updateNeighboor(EFace::Left, EFace::Right);
            }
        }
    }

	if(nbPointDisabled != 0)
	{
        // TO DO; only update needed part
		generateMesh(m_collisionProfileName, m_nbPoint - nbPointDisabled);
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
    if (DebugDrawProceduralMeshCollision == 1)
    {
        DrawDebugSphere(this->GetWorld(), _otherActor->GetActorLocation(), 70.0f, 12, FColor::Blue, false, 30.0f, 128, 10.0f);
        DrawDebugSphere(this->GetWorld(), _hit.ImpactPoint, 100.0f, 12, FColor::Red, false, 30.0f, 128, 10.0f);
    }

	// check if it's a bullet type
    hit(_hit.ImpactPoint);
}