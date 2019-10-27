// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldManager.h"
#include "EnvironmentManager.h"

// Sets default values
AWorldManager::AWorldManager()
	: m_nbChunck(3)
	, m_chunckSize(1000)
	, m_cubeSize(10)
	, BP_environment(nullptr)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
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

	for (int x = 0; x < m_nbChunck; ++x)
	{
		for (int y = 0; y < m_nbChunck; ++y)
		{
			for (int z = 0; z < m_nbChunck; ++z)
			{
				// new position
				FVector location(x * m_chunckSize, y * m_chunckSize, z * m_chunckSize);

				// add it to transform
				FTransform transform;
				transform.SetLocation(location);

				// start spawning actor
				AEnvironmentManager* BPEnvironment = world->SpawnActorDeferred<AEnvironmentManager>(BP_environment, transform);
				if (BPEnvironment)
				{
					// Init component
					BPEnvironment->init(FVector2D(location.X, location.X + m_chunckSize), FVector2D(location.Y, location.Y + m_chunckSize), FVector2D(location.Z, location.Z + m_chunckSize), m_cubeSize);
					BPEnvironment->FinishSpawning(transform);
				}
			}
		}
	}
}

