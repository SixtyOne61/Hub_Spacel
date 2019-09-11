// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldManager.h"
#include "EnvironmentManager.h"

// Sets default values
AWorldManager::AWorldManager()
	: m_nbChunck(10)
	, m_chunckSize(1000)
	, m_cubeSize(100)
	, BP_environment(nullptr)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	spawnChunckEnvironment();
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
					// TO DO : call init
					// Init component
					BPEnvironment->FinishSpawning(transform);
				}
			}
		}
	}
}

