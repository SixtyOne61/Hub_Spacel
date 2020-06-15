// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultShell.h"
#include "Components/SceneComponent.h"
#include "Source/Mesh/SpacelProceduralMeshComponent.h"
#include "Materials/MaterialInstance.h"

// Sets default values
ADefaultShell::ADefaultShell()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    ProceduralMesh = CreateDefaultSubobject<USpacelProceduralMeshComponent>(TEXT("ProceduralShell"));
    ProceduralMesh->bUseAsyncCooking = true;
    ProceduralMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ADefaultShell::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        SetReplicates(true);
        SetReplicateMovement(true);
    }
}

// Called every frame
void ADefaultShell::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ADefaultShell::GenerateMesh(TArray<FVector> const& _ignoreCoord)
{
    if (!ProceduralMesh)
    {
        return false;
    }

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
                chainedLocations.Add(MakeShareable(new ChainedLocation(loc, cubeSize)));
            }
        }
    }

    this->ProceduralMesh->setEdges(std::forward<TArray<TSharedPtr<ChainedLocation>>>(chainedLocations));
    this->ProceduralMesh->generateMesh(std::move(FName("Player")));
    this->ProceduralMesh->SetMaterial(0, this->Mat);
    return true;
}

