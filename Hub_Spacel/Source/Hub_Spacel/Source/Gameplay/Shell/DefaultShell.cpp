// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultShell.h"
#include "Source/Mesh/SpacelProceduralMeshComponent.h"
#include "Materials/MaterialInstance.h"

ADefaultShell::ADefaultShell()
    : AProceduralActor()
{

}

bool ADefaultShell::GenerateMesh(TArray<FVector> const& _ignoreCoord)
{
    if (!ensure(this->ProceduralMesh != nullptr)) return false;

    FVector cubeSize = FVector(15.0f, 15.0f, 15.0f);
    this->ProceduralMesh->setCubeSize(cubeSize);
    TArray<TSharedPtr<ChainedLocation>> chainedLocations;

    int8 radius = 105;
    for (int8 x = -radius; x < radius; x += 15)
    {
        for (int8 y = -radius; y < radius; y += 15)
        {
            for (int8 z = -radius; z < radius; z += 15)
            {
                FVector loc = FVector(x, y, z);
                if (FVector::Dist(loc, FVector::ZeroVector) >= radius)
                {
                    continue;
                }

                if (_ignoreCoord.Find(loc) != INDEX_NONE)
                {
                    continue;
                }
                chainedLocations.Add(MakeShareable(new ChainedLocation(loc, cubeSize, -1)));
            }
        }
    }

    this->ProceduralMesh->setEdges(std::forward<TArray<TSharedPtr<ChainedLocation>>>(chainedLocations));
    this->ProceduralMesh->generateMesh(std::move(FName("Player")));
    this->ProceduralMesh->SetMaterial(0, this->Mat);
    return true;
}

