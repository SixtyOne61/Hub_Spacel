// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldManager.h"
#include "EnvironmentManager.h"
#include "Components/BoxComponent.h"

// Sets default values
AWorldManager::AWorldManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Delimiter"));
    if (!ensure(BoxComponent != nullptr)) return;
    RootComponent = BoxComponent;
}

// Called when the game starts or when spawned
void AWorldManager::BeginPlay()
{
	Super::BeginPlay();
	
	spawnChunckEnvironment();
}

void AWorldManager::spawnChunckEnvironment()
{
	UWorld* const world = GetWorld();
    if (!ensure(world != nullptr)) return;

    // define chunck size by number of chunk
    if (!ensure(this->BoxComponent != nullptr)) return;
    FVector const& scaleBoxExtent = this->BoxComponent->GetScaledBoxExtent();
    FVector chunckSize = FVector(scaleBoxExtent.X / this->NbChunck,
                                scaleBoxExtent.Y / this->NbChunck,
                                scaleBoxExtent.Z / this->NbChunck);

    FVector offset = scaleBoxExtent;
    offset /= 2;

    // define cube size
    this->CubeSize = chunckSize / this->NbCubeByChunck;

    for (int x = 0; x < this->NbChunck; ++x) 
    {
        for (int y = 0; y < this->NbChunck; ++y)
        {
            for (int z = 0; z < this->NbChunck; ++z)
            {
                // new location
                FVector location(x * chunckSize.X - offset.X, y * chunckSize.Y - offset.Y, z * chunckSize.Z - offset.Z);

                // add it to transform
                FTransform transform;
                transform.SetLocation(location);

                // start spawning actor
                AEnvironmentManager* environment = world->SpawnActorDeferred<AEnvironmentManager>(this->EnvironmentClass, transform);
                if (environment)
                {
                    // init component
                    environment->Init(FVector2D(location.X, location.X + chunckSize.X), FVector2D(location.Y, location.Y + chunckSize.Y), FVector2D(location.Z, location.Z + chunckSize.Z), this->CubeSize);
                    environment->FinishSpawning(transform);
                }
            }
        }
    }
}

