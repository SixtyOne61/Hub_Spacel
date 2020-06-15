// Fill out your copyright notice in the Description page of Project Settings.


#include "HookComponent.h"
#include "Components/SceneComponent.h"
#include "Source/Mesh/SpacelProceduralMeshComponent.h"
#include "Materials/MaterialInstance.h"

AHookComponent::AHookComponent()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    ProceduralMesh = CreateDefaultSubobject<USpacelProceduralMeshComponent>(TEXT("ProceduralMesh"));
    ProceduralMesh->bUseAsyncCooking = true;
    ProceduralMesh->SetupAttachment(RootComponent);
}

void AHookComponent::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        SetReplicates(true);
        SetReplicateMovement(true);

        GenerateHook(135);
    }
}

void AHookComponent::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

bool AHookComponent::GenerateHook(float _innerRadius)
{
    FVector cubeSize = FVector(15.0f, 15.0f, 15.0f);
    this->ProceduralMesh->setCubeSize(cubeSize);
    
    TArray<TSharedPtr<ChainedLocation>> chainedLocations;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    for (uint16 deg = 0; deg <= 180; deg += 15)
    {
        float rad = FMath::DegreesToRadians(deg);
        y = _innerRadius * FMath::Cos(rad);
        z = _innerRadius * FMath::Sin(rad);

        FVector loc = FVector(x, y, z);
        chainedLocations.Add(MakeShareable(new ChainedLocation(loc, cubeSize)));
        
        if (z !=0)
        {
            loc = FVector(x, y, -z);
            chainedLocations.Add(MakeShareable(new ChainedLocation(loc, cubeSize)));
        }
    }
    /*
    int8 z = 0;
    for (int8 y = -_innerRadius; y <= _innerRadius; y += 15)
    {
        FVector loc = FVector(x, y, z);
        chainedLocations.Add(MakeShareable(new ChainedLocation(loc, cubeSize)));

        if (z != 0)
        {
            loc = FVector(x, y, -z);
            chainedLocations.Add(MakeShareable(new ChainedLocation(loc, cubeSize)));
        }

        z = (y > 0 ? z - 15 : z + 15);
    }*/

    this->ProceduralMesh->setEdges(std::forward<TArray<TSharedPtr<ChainedLocation>>>(chainedLocations));
    this->ProceduralMesh->generateMesh(std::move(FName("Player")));
    this->ProceduralMesh->SetMaterial(0, Mat);
    return true;
}
