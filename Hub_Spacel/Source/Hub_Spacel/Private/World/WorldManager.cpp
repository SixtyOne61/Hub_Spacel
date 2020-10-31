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

void AWorldManager::SpawnEnvironment() const
{
#ifdef WITH_EDITOR
    UWorld* const world = this->GetWorld();
    if (!ensure(world != nullptr)) return;

    // define chunck size by number of chunk
    if (!ensure(this->BoxComponent != nullptr)) return;
    int chunckSize = this->NbCubePerChunckPerAxis * this->CubeSize;
    int environmentSize = this->NbChunckPerAxis * this->NbCubePerChunckPerAxis * this->CubeSize;
    FVector box = FVector(environmentSize, environmentSize, environmentSize);
    this->BoxComponent->InitBoxExtent(box);

    FVector offset = box / 2.0f;

    for (int x = 0; x < this->NbChunckPerAxis; ++x)
    {
        for (int y = 0; y < this->NbChunckPerAxis; ++y)
        {
            for (int z = 0; z < this->NbChunckPerAxis; ++z)
            {
                // new location
                FVector location(x * chunckSize - offset.X, y * chunckSize - offset.Y, z * chunckSize - offset.Z);

                // add it to transform
                FTransform transform;
                transform.SetLocation(location);

                // start spawning actor
                AEnvironmentManager* environment = world->SpawnActorDeferred<AEnvironmentManager>(this->EnvironmentClass, transform);
                if (environment)
                {
                    // init component
                    environment->init(FVector2D(location.X, location.X + chunckSize), FVector2D(location.Y, location.Y + chunckSize), FVector2D(location.Z, location.Z + chunckSize), FVector(this->CubeSize, this->CubeSize, this->CubeSize));
                    environment->FinishSpawning(transform);
                }
            }
        }
    }
#endif // WITH_EDITOR
}

// Called when the game starts or when spawned
void AWorldManager::BeginPlay()
{
	Super::BeginPlay();
}


