// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralActor.h"
#include "Components/SceneComponent.h"
#include "Source/Mesh/SpacelProceduralMeshComponent.h"
#include "Materials/MaterialInstance.h"

// Sets default values
AProceduralActor::AProceduralActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    ProceduralMesh = CreateDefaultSubobject<USpacelProceduralMeshComponent>(TEXT("ProceduralMesh"));
    ProceduralMesh->bUseAsyncCooking = true;
    ProceduralMesh->SetupAttachment(RootComponent);
}

bool AProceduralActor::GenerateMesh(TArray<FVector> const& _ignoreCoord)
{
    return false;
}

// Called when the game starts or when spawned
void AProceduralActor::BeginPlay()
{
	Super::BeginPlay();

    if (HasAuthority())
    {
        SetReplicates(true);
        SetReplicateMovement(true);
    }
}

// Called every frame
void AProceduralActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

