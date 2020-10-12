// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultEngine.h"
#include "Components/SceneComponent.h"
#include "Source/Mesh/SpacelProceduralMeshComponent.h"
#include "Materials/MaterialInstance.h"

// Sets default values
ADefaultEngine::ADefaultEngine()
    : AProceduralActor()
{
}

// Called when the game starts or when spawned
void ADefaultEngine::BeginPlay()
{
	Super::BeginPlay();
}

bool ADefaultEngine::GenerateMesh(TArray<FVector> const& _ignoreCoord)
{
    if (!ensure(this->ProceduralMesh != nullptr)) return false;

    FVector cubeSize = FVector(15.0f, 15.0f, 15.0f);
    this->ProceduralMesh->setCubeSize(cubeSize);
    // TO DO : expose
    int8 radius = 120;
    TArray<TSharedPtr<ChainedLocation>> chainedLocations =
    {
        MakeShareable(new ChainedLocation(FVector(-60.0f, -radius, 0.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-45.0f, -radius, 0.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-30.0f, -radius, 0.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-15.0f, -radius, 0.0f), cubeSize, -1)),

        MakeShareable(new ChainedLocation(FVector(-75.0f, -radius + 15.0f, 0.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-90.0f, -radius + 15.0f, 0.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-105.0f, -radius + 30.0f, 0.0f), cubeSize, -1)),

        MakeShareable(new ChainedLocation(FVector(-60.0f, radius, 0.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-45.0f, radius, 0.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-30.0f, radius, 0.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-15.0f, radius, 0.0f), cubeSize, -1)),

        MakeShareable(new ChainedLocation(FVector(-75.0f, radius - 15.0f, 0.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-90.0f, radius - 15.0f, 0.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-105.0f, radius - 30.0f, 0.0f), cubeSize, -1)),

        MakeShareable(new ChainedLocation(FVector(-120.0f, -radius + 30.0f, 0.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, -radius + 45.0f, 0.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, -radius + 60.0f, 0.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, -radius + 60.0f, 15.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, -radius + 60.0f, -15.0f), cubeSize, -1)),

        MakeShareable(new ChainedLocation(FVector(-120.0f, radius - 30.0f, 0.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, radius - 45.0f, 0.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, radius - 60.0f, 0.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, radius - 60.0f, 15.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, radius - 60.0f, -15.0f), cubeSize, -1)),
    };

    this->ProceduralMesh->setEdges(std::forward<TArray<TSharedPtr<ChainedLocation>>>(chainedLocations));
    this->ProceduralMesh->generateMesh(std::move(FName("Player")));
    this->ProceduralMesh->SetMaterial(0, this->Mat);
    return true;
}

