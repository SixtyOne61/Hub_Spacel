// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldManager.h"
#include "EnvironmentManager.h"
#include "Components/BoxComponent.h"

// Sets default values
AWorldManager::AWorldManager()
    : NbChunck(3)
    , ChunckSize(1000)
    , CubeSize(10)
    , EnvironmentClass(nullptr)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    Delimiter = CreateDefaultSubobject<UBoxComponent>(TEXT("Delimitier"));
    RootComponent = Delimiter;
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
	if (!world)
	{
		return;
	}

    // define chunck size by number of chunk
    FVector chunckSize = FVector(Delimiter->GetScaledBoxExtent().X / NbChunck,
        Delimiter->GetScaledBoxExtent().Y / NbChunck,
        Delimiter->GetScaledBoxExtent().Z / NbChunck);

	for (int x = 0; x < this->NbChunck; ++x)
	{
		for (int y = 0; y < this->NbChunck; ++y)
		{
			for (int z = 0; z < this->NbChunck; ++z)
			{
				// new position
				FVector location(x * this->ChunckSize, y * this->ChunckSize, z * this->ChunckSize);

				// add it to transform
				FTransform transform;
				transform.SetLocation(location);

				// start spawning actor
				AEnvironmentManager* environment = world->SpawnActorDeferred<AEnvironmentManager>(EnvironmentClass, transform);
				if (environment)
				{
					// Init component
                    environment->init(FVector2D(location.X, location.X + this->ChunckSize), FVector2D(location.Y, location.Y + this->ChunckSize), FVector2D(location.Z, location.Z + this->ChunckSize), this->CubeSize);
                    environment->FinishSpawning(transform);
				}
			}
		}
	}
}

