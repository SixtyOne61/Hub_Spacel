// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvironmentManager.h"
#include "SimplexNoise/Public/SimplexNoiseBPLibrary.h"

// Sets default values
AEnvironmentManager::AEnvironmentManager()
	: m_bornX(1000)
	, m_bornY(1000)
	, m_bornZ(1000)
	, m_cubeSize(10)
	, m_noise()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AEnvironmentManager::BeginPlay()
{
	Super::BeginPlay();
	
	// create procedural world
	createProceduralWorld();
}

// Called every frame
void AEnvironmentManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

TSharedPtr<ChainedLocation> AEnvironmentManager::createChain(FVector& _location, TArray<FVector>& _openList) const
{
	TSharedPtr<ChainedLocation> newPos = MakeShareable(new ChainedLocation(std::forward<FVector>(_location), m_cubeSize));
	addNeighboor(_openList, _location + FVector(0, m_cubeSize, 0), EFace::Top, newPos, EFace::Bot);
	addNeighboor(_openList, _location + FVector(0, -m_cubeSize, 0), EFace::Bot, newPos, EFace::Top);
	addNeighboor(_openList, _location + FVector(m_cubeSize, 0, 0), EFace::Right, newPos, EFace::Left);
	addNeighboor(_openList, _location + FVector(-m_cubeSize, 0, 0), EFace::Left, newPos, EFace::Right);
	addNeighboor(_openList, _location + FVector(0, 0, m_cubeSize), EFace::Back, newPos, EFace::Front);
	addNeighboor(_openList, _location + FVector(0, 0, -m_cubeSize), EFace::Front, newPos, EFace::Back);
	return newPos;
}

void AEnvironmentManager::createProceduralWorld() const
{
	TArray<FVector> openList;

	for (int x = 0; x <= m_bornX; x += m_cubeSize)
	{
		for (int y = 0; y <= m_bornY; y += m_cubeSize)
		{
			for (int z = 0; z <= m_bornZ; z += m_cubeSize)
			{
				if (isValidNoise(x, y, z))
				{
					openList.Add(FVector(x, y, z));
				}
			}
		}
	}

	while (openList.Num())
	{
		FVector location = openList[0];
		openList.RemoveAt(0);
		createChain(location, openList);
	}
}

void AEnvironmentManager::addNeighboor(TArray<FVector>& _openList, FVector _location, EFace _where, TSharedPtr<ChainedLocation> _chain, EFace _inverse) const
{
	if (isValidLocation(_location) && isValidNoise(_location))
	{
		_openList.Remove(_location);
		TSharedPtr<ChainedLocation> newPos = createChain(_location, _openList);
		_chain->addNeighbor(_where, newPos);
		newPos->addNeighbor(_inverse, _chain);
	}
}

bool AEnvironmentManager::isValidNoise(int _x, int _y, int _z) const
{
	float noise = 0.0f; // USimplexNoiseBPLibrary::SimplexNoise3D(_x, _y, _z);
	return noise >= 0.7f;
}

bool AEnvironmentManager::isValidNoise(FVector const& _location) const
{
	float noise = 0.0f; // USimplexNoiseBPLibrary::SimplexNoise3D(_location.X, _location.Y, _location.Z);
	return noise >= 0.7f;
}

bool AEnvironmentManager::isValidLocation(FVector const& _location) const
{
	return _location.X >= 0 && _location.X <= m_bornX && _location.Y >= 0 && _location.Y <= m_bornY && _location.Z >= 0 && _location.Z <= m_bornZ;
}