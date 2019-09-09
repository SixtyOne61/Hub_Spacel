// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvironmentManager.h"
#include "Hub_Spacel/Source/World/Asteroid.h"

// Sets default values
AEnvironmentManager::AEnvironmentManager()
	: m_bornX(1000)
	, m_bornY(1000)
	, m_bornZ(1000)
	, m_cubeSize(10)
	, BP_asteroid(nullptr)
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

void AEnvironmentManager::createProceduralWorld()
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
		
		// spawn BP
		spawnAsteroid();
	}
}

TSharedPtr<ChainedLocation> AEnvironmentManager::createChain(FVector& _location, TArray<FVector>& _openList)
{
	TSharedPtr<ChainedLocation> newPos = MakeShareable(new ChainedLocation(std::forward<FVector>(_location), m_cubeSize));

	m_currentObject.Add(newPos);

	addNeighboor(_openList, _location + FVector(0, m_cubeSize, 0), EFace::Top, newPos, EFace::Bot);
	addNeighboor(_openList, _location + FVector(0, -m_cubeSize, 0), EFace::Bot, newPos, EFace::Top);
	addNeighboor(_openList, _location + FVector(m_cubeSize, 0, 0), EFace::Right, newPos, EFace::Left);
	addNeighboor(_openList, _location + FVector(-m_cubeSize, 0, 0), EFace::Left, newPos, EFace::Right);
	addNeighboor(_openList, _location + FVector(0, 0, m_cubeSize), EFace::Back, newPos, EFace::Front);
	addNeighboor(_openList, _location + FVector(0, 0, -m_cubeSize), EFace::Front, newPos, EFace::Back);
	return newPos;
}

void AEnvironmentManager::addNeighboor(TArray<FVector>& _openList, FVector _location, EFace _where, TSharedPtr<ChainedLocation> _chain, EFace _inverse)
{
	if (isValidLocation(_location) && isValidNoise(_location))
	{
		// two way, check if it's a known value or create a new one
		TSharedPtr<ChainedLocation> exist = isKnownLocation(_location);
		if (exist)
		{
			_chain->addNeighbor(_where, exist);
			exist->addNeighbor(_inverse, _chain);
		}
		else if (_openList.Contains(_location))
		{
			_openList.Remove(_location);
			TSharedPtr<ChainedLocation> newPos = createChain(_location, _openList);
		}
		else
		{
			// TO DO : throw error
		}
	}
}

void AEnvironmentManager::spawnAsteroid()
{
	UWorld* const world = GetWorld();
	if (world) 
	{
		FVector location = FVector(0, 0, 0);

		/*FActorSpawnParameters spawnParams;
		spawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		spawnParams.Instigator = GetInstigator();
		spawnParams.Owner = GetOwner();*/

		FTransform transform;
		transform.SetLocation(location);

		AAsteroid* BPasteroid = world->SpawnActorDeferred<AAsteroid>(BP_asteroid, transform);
		if (BPasteroid) 
		{
			// Init component
			BPasteroid->setCubeSize(m_cubeSize);
			BPasteroid->setEdges(std::forward<TArray<TSharedPtr<ChainedLocation>>>(m_currentObject));
			BPasteroid->FinishSpawning(transform);
			m_currentObject.Empty();
		}
	}
}

bool AEnvironmentManager::isValidNoise(int _x, int _y, int _z) const
{
	float noise = m_noise.getOctaveNoise(_x * 0.00020f, _y * 0.00020f, _z * 0.00020f, 4);
	//UE_LOG(LogTemp, Warning, TEXT(*FString::FromInt(noise)));
	return noise >= .75f;
}

bool AEnvironmentManager::isValidNoise(FVector const& _location) const
{
	return isValidNoise(_location.X, _location.Y, _location.Z);
}

bool AEnvironmentManager::isValidLocation(FVector const& _location) const
{
	return _location.X >= 0 && _location.X <= m_bornX && _location.Y >= 0 && _location.Y <= m_bornY && _location.Z >= 0 && _location.Z <= m_bornZ;
}

TSharedPtr<ChainedLocation> AEnvironmentManager::isKnownLocation(FVector const& _location) const
{
	for (TSharedPtr<ChainedLocation> const& value : m_currentObject)
	{
		if (value->getCenter() == _location)
		{
			return value;
		}
	}

	return nullptr;
}