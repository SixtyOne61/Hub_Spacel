// Fill out your copyright notice in the Description page of Project Settings.


#include "Hook.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Source/Mesh/SpacelProceduralMeshComponent.h"
#include "Materials/MaterialInstance.h"
#include "Hub_Pawn.h"

AHook::AHook()
    : AProceduralActor()
{
    BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
    if (!ensure(BoxComponent != nullptr)) return;
    BoxComponent->ComponentTags.Add(FName(TEXT("HookBox")));
    BoxComponent->SetupAttachment(RootComponent);
}

void AHook::BeginPlay()
{
    Super::BeginPlay();

    if (this->HasAuthority())
    {
        if (!ensure(this->BoxComponent != nullptr)) return;
        this->BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AHook::OnBeginOverlap);
    }
}

bool AHook::GenerateMesh(TArray<FVector> const& _ignoreCoord)
{
    if (!ensure(this->ProceduralMesh != nullptr)) return false;

    FVector cubeSize = FVector(15.0f, 15.0f, 15.0f);
    this->ProceduralMesh->setCubeSize(cubeSize);

    TArray<TSharedPtr<ChainedLocation>> chainedLocations;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    for (uint16 deg = 30; deg <= 150; deg += 15)
    {
        float rad = FMath::DegreesToRadians(deg);
        y = this->InnerRadius * FMath::Cos(rad);
        z = this->InnerRadius * FMath::Sin(rad);

        FVector loc = FVector(x, y, z);
        chainedLocations.Add(MakeShareable(new ChainedLocation(loc, cubeSize)));
    }

    float maxZ = this->InnerRadius * FMath::Sin(FMath::DegreesToRadians(90));
    float radius = this->InnerRadius / 2.0f;
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
    this->ProceduralMesh->SetMaterial(0, this->Mat);
    this->ProceduralMesh->SetCollisionProfileName("NoCollision");

    return true;
}

void AHook::OnBeginOverlap(class UPrimitiveComponent* _overlappedComponent, class AActor* _otherActor, class UPrimitiveComponent* _otherComp, int32 _otherBodyIndex, bool _bFromSweep, const FHitResult& _sweepResult)
{
    if (!ensure(_otherActor != nullptr)) return;
    if (!ensure(_otherComp != nullptr)) return;

    if (_overlappedComponent->ComponentHasTag(FName(TEXT("HookBox")))
        && _otherComp->ComponentHasTag(FName(TEXT("RodBox"))))
    {
        if (AHub_Pawn * parent = Cast<AHub_Pawn>(this->GetParentActor()))
        {
            parent->SetHook(_otherActor);
        }
    }
}
