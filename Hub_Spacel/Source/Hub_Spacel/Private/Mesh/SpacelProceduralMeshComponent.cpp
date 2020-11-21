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

void USpacelProceduralMeshComponent::BeginPlay()
{
    Super::BeginPlay();
    OnComponentHit.AddDynamic(this, &USpacelProceduralMeshComponent::onHit);
}

void USpacelProceduralMeshComponent::generateMesh(FName _collisionProfileName, int _nbPoint, TArray<FLocationInformation> & _locations)
{
    // clear all
    this->ClearAllMeshSections();
    this->ClearCollisionConvexMeshes();

    FVector halfCubeSize = this->CubeSize / 2;

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

	for (FLocationInformation & point : _locations)
	{
        if (!point.Used)
        {
            continue;
        }

		FVector const& center = point.Location;

		// read mask from linkPos
		{
            int deb = vertices.Num();
            vertices.Add(center + FVector(-halfCubeSize.X, halfCubeSize.Y, -halfCubeSize.Z));
            vertices.Add(center + FVector(halfCubeSize.X, halfCubeSize.Y, -halfCubeSize.Z));
            vertices.Add(center + FVector(halfCubeSize.X, halfCubeSize.Y, halfCubeSize.Z));
            vertices.Add(center + FVector(-halfCubeSize.X, halfCubeSize.Y, halfCubeSize.Z));

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
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(halfCubeSize.X, -halfCubeSize.Y, halfCubeSize.Z));
			vertices.Add(center + FVector(halfCubeSize.X, -halfCubeSize.Y, -halfCubeSize.Z));
			vertices.Add(center + FVector(-halfCubeSize.X, -halfCubeSize.Y, -halfCubeSize.Z));
            vertices.Add(center + FVector(-halfCubeSize.X, -halfCubeSize.Y, halfCubeSize.Z));

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
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(halfCubeSize.X, -halfCubeSize.Y, halfCubeSize.Z));
			vertices.Add(center + FVector(halfCubeSize.X, halfCubeSize.Y, halfCubeSize.Z));
			vertices.Add(center + FVector(halfCubeSize.X, halfCubeSize.Y, -halfCubeSize.Z));
            vertices.Add(center + FVector(halfCubeSize.X, -halfCubeSize.Y, -halfCubeSize.Z));

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
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(-halfCubeSize.X, -halfCubeSize.Y, halfCubeSize.Z));
			vertices.Add(center + FVector(-halfCubeSize.X, -halfCubeSize.Y, -halfCubeSize.Z));
			vertices.Add(center + FVector(-halfCubeSize.X, halfCubeSize.Y, -halfCubeSize.Z));
            vertices.Add(center + FVector(-halfCubeSize.X, halfCubeSize.Y, halfCubeSize.Z));

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
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(-halfCubeSize.X, halfCubeSize.Y, -halfCubeSize.Z));
			vertices.Add(center + FVector(-halfCubeSize.X, -halfCubeSize.Y, -halfCubeSize.Z));
			vertices.Add(center + FVector(halfCubeSize.X, -halfCubeSize.Y, -halfCubeSize.Z));
            vertices.Add(center + FVector(halfCubeSize.X, halfCubeSize.Y, -halfCubeSize.Z));

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
		{
			int deb = vertices.Num();
			vertices.Add(center + FVector(halfCubeSize.X, -halfCubeSize.Y, halfCubeSize.Z));
			vertices.Add(center + FVector(-halfCubeSize.X, -halfCubeSize.Y, halfCubeSize.Z));
			vertices.Add(center + FVector(-halfCubeSize.X, halfCubeSize.Y, halfCubeSize.Z));
            vertices.Add(center + FVector(halfCubeSize.X, halfCubeSize.Y, halfCubeSize.Z));

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
	this->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    this->SetCollisionProfileName(_collisionProfileName);
	bUseComplexAsSimpleCollision = true;
	this->SetNotifyRigidBodyCollision(true);

	this->CreateMeshSection_LinearColor(0, vertices, triangles, normals, UV0, vertexColors, {}, true);

	// Enable collision data
	this->AddCollisionConvexMesh(vertices);
}

bool USpacelProceduralMeshComponent::hit(FVector const& _impactPoint)
{
    // on peut peut être faire une section avec la couche extérieur du mesh, et chercher dans cette couche
    // ensuite nous devrions taper dans l'a couche inferieur pour chercher qui passe en couche sup ?
    //this->GetProcMeshSection(0)->ProcVertexBuffer[0].Position !!
    // for this moment we keep cubeSize instead half (more impact size)
    float radius = this->CubeSize.Size() / 2;

    FProcMeshSection* procMeshSection = this->GetProcMeshSection(0);
    if (procMeshSection)
    {
        //DrawDebugSphere(this->GetWorld(), procMeshSection->ProcVertexBuffer[0].Position, 100.0f, 12, FColor::Blue, false, 30.0f, 128, 10.0f);
        int32 nbRemove = procMeshSection->ProcVertexBuffer.RemoveAll([&](auto const _vert) -> bool
        {
            return FVector::Dist(_vert.Position + this->OwnerLocation, _impactPoint) <= radius;
        });

        if (nbRemove != 0)
        {
            int32 size = procMeshSection->ProcVertexBuffer.Num();

            TArray<FVector> vertices, normals;
            vertices.Reserve(size);
            normals.Reserve(size);

            TArray<FVector2D> uv0;
            uv0.Reserve(size);

            TArray<FLinearColor> linearColors;
            linearColors.Reserve(size);

            TArray<FProcMeshTangent> tangents;
            tangents.Reserve(size);

            for (auto const& vertex : procMeshSection->ProcVertexBuffer)
            {
                vertices.Add(vertex.Position);
                normals.Add(vertex.Normal);
                uv0.Add(vertex.UV0);
                linearColors.Add(vertex.Color);
                tangents.Add(vertex.Tangent);
            }
            this->UpdateMeshSection_LinearColor(0, vertices, normals, uv0, linearColors, tangents);

            return true;
        }
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
    //DrawDebugSphere(this->GetWorld(), _hit.ImpactPoint, 100.0f, 12, FColor::Red, false, 30.0f, 128, 10.0f);

	// check if it's a bullet type
    hit(_hit.ImpactPoint);
}

void USpacelProceduralMeshComponent::generateVoxelMesh(int32 _cubeSize)
{
    // clear all
    this->ClearAllMeshSections();
    this->ClearCollisionConvexMeshes();

    FVector halfCubeSize = FVector(_cubeSize / 2, _cubeSize / 2, _cubeSize / 2);

    TArray<int32> triangles;
    triangles.Reserve(6);

    int maxSize = 24;
    TArray<FVector> vertices;
    vertices.Reserve(maxSize);
    TArray<FVector> normals;
    normals.Reserve(maxSize);
    TArray<FLinearColor> vertexColors;
    vertexColors.Reserve(maxSize);
    TArray<FVector2D> UV0;
    UV0.Reserve(maxSize);

    FVector center { 0, 0, 0 };

    // read mask from linkPos
    {
        int deb = vertices.Num();
        vertices.Add(center + FVector(-halfCubeSize.X, halfCubeSize.Y, -halfCubeSize.Z));
        vertices.Add(center + FVector(halfCubeSize.X, halfCubeSize.Y, -halfCubeSize.Z));
        vertices.Add(center + FVector(halfCubeSize.X, halfCubeSize.Y, halfCubeSize.Z));
        vertices.Add(center + FVector(-halfCubeSize.X, halfCubeSize.Y, halfCubeSize.Z));

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
    {
        int deb = vertices.Num();
        vertices.Add(center + FVector(halfCubeSize.X, -halfCubeSize.Y, halfCubeSize.Z));
        vertices.Add(center + FVector(halfCubeSize.X, -halfCubeSize.Y, -halfCubeSize.Z));
        vertices.Add(center + FVector(-halfCubeSize.X, -halfCubeSize.Y, -halfCubeSize.Z));
        vertices.Add(center + FVector(-halfCubeSize.X, -halfCubeSize.Y, halfCubeSize.Z));

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
    {
        int deb = vertices.Num();
        vertices.Add(center + FVector(halfCubeSize.X, -halfCubeSize.Y, halfCubeSize.Z));
        vertices.Add(center + FVector(halfCubeSize.X, halfCubeSize.Y, halfCubeSize.Z));
        vertices.Add(center + FVector(halfCubeSize.X, halfCubeSize.Y, -halfCubeSize.Z));
        vertices.Add(center + FVector(halfCubeSize.X, -halfCubeSize.Y, -halfCubeSize.Z));

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
    {
        int deb = vertices.Num();
        vertices.Add(center + FVector(-halfCubeSize.X, -halfCubeSize.Y, halfCubeSize.Z));
        vertices.Add(center + FVector(-halfCubeSize.X, -halfCubeSize.Y, -halfCubeSize.Z));
        vertices.Add(center + FVector(-halfCubeSize.X, halfCubeSize.Y, -halfCubeSize.Z));
        vertices.Add(center + FVector(-halfCubeSize.X, halfCubeSize.Y, halfCubeSize.Z));

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
    {
        int deb = vertices.Num();
        vertices.Add(center + FVector(-halfCubeSize.X, halfCubeSize.Y, -halfCubeSize.Z));
        vertices.Add(center + FVector(-halfCubeSize.X, -halfCubeSize.Y, -halfCubeSize.Z));
        vertices.Add(center + FVector(halfCubeSize.X, -halfCubeSize.Y, -halfCubeSize.Z));
        vertices.Add(center + FVector(halfCubeSize.X, halfCubeSize.Y, -halfCubeSize.Z));

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
    {
        int deb = vertices.Num();
        vertices.Add(center + FVector(halfCubeSize.X, -halfCubeSize.Y, halfCubeSize.Z));
        vertices.Add(center + FVector(-halfCubeSize.X, -halfCubeSize.Y, halfCubeSize.Z));
        vertices.Add(center + FVector(-halfCubeSize.X, halfCubeSize.Y, halfCubeSize.Z));
        vertices.Add(center + FVector(halfCubeSize.X, halfCubeSize.Y, halfCubeSize.Z));

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

    // setup collision
    this->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    this->SetCollisionProfileName("BlockAll");
    bUseComplexAsSimpleCollision = true;
    this->SetNotifyRigidBodyCollision(true);

    this->CreateMeshSection_LinearColor(0, vertices, triangles, normals, UV0, vertexColors, {}, true);

    // Enable collision data
    this->AddCollisionConvexMesh(vertices);
}