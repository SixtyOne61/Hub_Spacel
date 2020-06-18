// Fill out your copyright notice in the Description page of Project Settings.

#include "Rod.h"
#include "Source/Mesh/SpacelProceduralMeshComponent.h"
#include "Materials/MaterialInstance.h"

// Sets default values
ARod::ARod()
    : AProceduralActor()
{

}

// Called when the game starts or when spawned
void ARod::BeginPlay()
{
	Super::BeginPlay();
	
}

bool ARod::GenerateMesh(TArray<FVector> const& _ignoreCoord)
{
    if (!ProceduralMesh)
    {
        return false;
    }

    FVector cubeSize = FVector(15.0f, 15.0f, 15.0f);
    this->ProceduralMesh->setCubeSize(cubeSize);

    TArray<TSharedPtr<ChainedLocation>> chainedLocations =
    {
        MakeShareable(new ChainedLocation(FVector(-120.0f, 15.0f, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, -15.0f, 0.0f), cubeSize)),

        MakeShareable(new ChainedLocation(FVector(-120.0f, 15.0f, 15.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, -15.0f, 15.0f), cubeSize)),

        MakeShareable(new ChainedLocation(FVector(-120.0f, 15.0f, -15.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, -15.0f, -15.0f), cubeSize)),

        MakeShareable(new ChainedLocation(FVector(-120.0f, 0.0f, 30.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, 15.0f, 30.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, -15.0f, 30.0f), cubeSize)),

        MakeShareable(new ChainedLocation(FVector(-120.0f, 0.0f, -30.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, 15.0f, -30.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, -15.0f, -30.0f), cubeSize)),
    };

    this->ProceduralMesh->setEdges(std::forward<TArray<TSharedPtr<ChainedLocation>>>(chainedLocations));
    this->ProceduralMesh->generateMesh(std::move(FName("Player")));
    this->ProceduralMesh->SetMaterial(0, Mat);
    return true;
}

