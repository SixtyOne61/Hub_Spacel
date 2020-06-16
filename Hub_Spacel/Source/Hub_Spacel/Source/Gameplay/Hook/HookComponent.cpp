// Fill out your copyright notice in the Description page of Project Settings.


#include "HookComponent.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
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

    BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
    BoxComponent->SetupAttachment(RootComponent);
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
    if (!ensure(ProceduralMesh != nullptr)) return false;

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

    float maxZ = _innerRadius * FMath::Sin(FMath::DegreesToRadians(90));
    float radius = _innerRadius / 2.0f;
    for (uint16 deg = 30; deg <= 150; deg += 15)
    {
        float rad = FMath::DegreesToRadians(deg);
        y = radius * FMath::Cos(rad);
        z = maxZ + radius * FMath::Sin(rad);

        FVector loc = FVector(x, y, z);
        chainedLocations.Add(MakeShareable(new ChainedLocation(loc, cubeSize)));
    }

    this->ProceduralMesh->setEdges(std::forward<TArray<TSharedPtr<ChainedLocation>>>(chainedLocations));
    this->ProceduralMesh->generateMesh(std::move(FName("Player")));
    this->ProceduralMesh->SetMaterial(0, Mat);

    // init box
    if (!ensure(BoxComponent != nullptr)) return false;

    BoxComponent->SetBoxExtent(FVector(5.0f, radius, radius * FMath::Sin(FMath::DegreesToRadians(90))), true);
    BoxComponent->SetRelativeLocation(FVector(0.0f, -radius/2.0f, maxZ));

    return true;
}
