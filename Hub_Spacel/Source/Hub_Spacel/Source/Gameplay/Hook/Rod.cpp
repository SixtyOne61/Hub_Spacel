// Fill out your copyright notice in the Description page of Project Settings.

#include "Rod.h"
#include "Source/Mesh/SpacelProceduralMeshComponent.h"
#include "Materials/MaterialInstance.h"
#include "Components/BoxComponent.h"

// Sets default values
ARod::ARod()
    : AProceduralActor()
{
    BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
    if (!ensure(BoxComponent != nullptr)) return;
    BoxComponent->ComponentTags.Add(FName(TEXT("RodBox")));
    BoxComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ARod::BeginPlay()
{
	Super::BeginPlay();
}

bool ARod::GenerateMesh(TArray<FVector> const& _ignoreCoord)
{
    if (!ensure(this->ProceduralMesh != nullptr)) return false;

    FVector cubeSize = FVector(15.0f, 15.0f, 15.0f);
    this->ProceduralMesh->setCubeSize(cubeSize);

    TArray<TSharedPtr<ChainedLocation>> chainedLocations =
    {
        MakeShareable(new ChainedLocation(FVector(30.0f, 0.0f, -120.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(15.0f, 0.0f, -120.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(0.0f, 0.0f, -120.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-15.0f, 0.0f, -120.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-30.0f, 0.0f, -120.0f), cubeSize, -1)),
        
        MakeShareable(new ChainedLocation(FVector(30.0f, 0.0f, -135.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(15.0f, 0.0f, -135.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(0.0f, 0.0f, -135.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-15.0f, 0.0f, -135.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-30.0f, 0.0f, -135.0f), cubeSize, -1)),

        MakeShareable(new ChainedLocation(FVector(15.0f, 0.0f, -150.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(0.0f, 0.0f, -150.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-15.0f, 0.0f, -150.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-30.0f, 0.0f, -150.0f), cubeSize, -1)),

        MakeShareable(new ChainedLocation(FVector(15.0f, 0.0f, -165.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(0.0f, 0.0f, -165.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-15.0f, 0.0f, -165.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-30.0f, 0.0f, -165.0f), cubeSize, -1)),

        MakeShareable(new ChainedLocation(FVector(0.0f, 0.0f, -180.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-15.0f, 0.0f, -180.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-30.0f, 0.0f, -180.0f), cubeSize, -1)),

        MakeShareable(new ChainedLocation(FVector(0.0f, 0.0f, -195.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-15.0f, 0.0f, -195.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-30.0f, 0.0f, -195.0f), cubeSize, -1)),

        MakeShareable(new ChainedLocation(FVector(0.0f, 0.0f, -210.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-15.0f, 0.0f, -210.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-30.0f, 0.0f, -210.0f), cubeSize, -1)),

        MakeShareable(new ChainedLocation(FVector(-15.0f, 0.0f, -225.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-30.0f, 0.0f, -225.0f), cubeSize, -1)),

        MakeShareable(new ChainedLocation(FVector(-15.0f, 0.0f, -240.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-30.0f, 0.0f, -240.0f), cubeSize, -1)),

        MakeShareable(new ChainedLocation(FVector(-30.0f, 0.0f, -255.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-30.0f, 0.0f, -270.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-30.0f, 0.0f, -285.0f), cubeSize, -1)),
        MakeShareable(new ChainedLocation(FVector(-30.0f, 0.0f, -300.0f), cubeSize, -1)),
    };

    this->ProceduralMesh->setEdges(std::forward<TArray<TSharedPtr<ChainedLocation>>>(chainedLocations));
    this->ProceduralMesh->generateMesh(std::move(FName("Player"))); // TO DO : change profile ?
    this->ProceduralMesh->SetMaterial(0, this->Mat);

    return true;
}

