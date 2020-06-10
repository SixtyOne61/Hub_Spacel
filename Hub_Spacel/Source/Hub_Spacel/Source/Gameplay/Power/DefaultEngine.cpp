// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultEngine.h"
#include "Components/SceneComponent.h"
#include "Source/Mesh/SpacelProceduralMeshComponent.h"
#include "Materials/MaterialInstance.h"

// Sets default values
ADefaultEngine::ADefaultEngine()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    ProceduralMesh = CreateDefaultSubobject<USpacelProceduralMeshComponent>(TEXT("ProceduralMesh"));
    ProceduralMesh->bUseAsyncCooking = true;
    ProceduralMesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ADefaultEngine::BeginPlay()
{
	Super::BeginPlay();

    if (HasAuthority())
    {
        SetReplicates(true);
        SetReplicateMovement(true);
    }
}

// Called every frame
void ADefaultEngine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ADefaultEngine::GenerateMesh(TArray<FVector> const& _ignoreCoord)
{
    if (!ProceduralMesh)
    {
        return false;
    }

    FVector cubeSize = FVector(15.0f, 15.0f, 15.0f);
    this->ProceduralMesh->setCubeSize(cubeSize);
    int8 radius = 120;
    TArray<TSharedPtr<ChainedLocation>> chainedLocations =
    {
        MakeShareable(new ChainedLocation(FVector(-60.0f, -radius, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-45.0f, -radius, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-30.0f, -radius, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-15.0f, -radius, 0.0f), cubeSize)),

        MakeShareable(new ChainedLocation(FVector(-75.0f, -radius + 15.0f, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-90.0f, -radius + 15.0f, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-105.0f, -radius + 30.0f, 0.0f), cubeSize)),

        MakeShareable(new ChainedLocation(FVector(-60.0f, radius, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-45.0f, radius, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-30.0f, radius, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-15.0f, radius, 0.0f), cubeSize)),

        MakeShareable(new ChainedLocation(FVector(-75.0f, radius - 15.0f, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-90.0f, radius - 15.0f, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-105.0f, radius - 30.0f, 0.0f), cubeSize)),

        MakeShareable(new ChainedLocation(FVector(-120.0f, -radius + 30.0f, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, -radius + 45.0f, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, -radius + 60.0f, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, -radius + 60.0f, 15.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, -radius + 60.0f, -15.0f), cubeSize)),

        MakeShareable(new ChainedLocation(FVector(-120.0f, radius - 30.0f, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, radius - 45.0f, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, radius - 60.0f, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, radius - 60.0f, 15.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, radius - 60.0f, -15.0f), cubeSize)),
    };

    this->ProceduralMesh->setEdges(std::forward<TArray<TSharedPtr<ChainedLocation>>>(chainedLocations));
    this->ProceduralMesh->generateMesh(std::move(FName("Player")));
    this->ProceduralMesh->SetMaterial(0, Mat);
    return true;
}

