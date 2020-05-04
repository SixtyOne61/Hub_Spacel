// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvironmentManager.h"
#include "Hub_Spacel/Source/Noise/SpacelNoise.h"
#include "Materials/MaterialInstance.h"
#include "Hub_Spacel/Source/Mesh/SpacelProceduralMeshComponent.h"

// Sets default values
AEnvironmentManager::AEnvironmentManager()
	: BornX()
	, BornY()
	, BornZ()
	, CubeSize(FVector::ZeroVector)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(true);
}

void AEnvironmentManager::init(FVector2D const& _bornX, FVector2D const& _bornY, FVector2D const& _bornZ, FVector const& _cubeSize)
{
	BornX = _bornX;
	BornY = _bornY;
	BornZ = _bornZ;
	CubeSize = _cubeSize;
}

// Called when the game starts or when spawned
void AEnvironmentManager::BeginPlay()
{
	Super::BeginPlay();

	// create procedural world
	createProceduralWorld();

	// init all procedural mesh
	for (auto proceduralMesh : m_proceduralMeshComponents)
	{
		if (!proceduralMesh)
		{
			continue;
		}

		proceduralMesh->setOwnerLocation(GetActorLocation());
        proceduralMesh->generateMesh(std::move(FName("BlockAll")));
        if (MatAsteroid)
        {
            proceduralMesh->SetMaterial(0, MatAsteroid);
        }
	}
}

void AEnvironmentManager::createProceduralWorld()
{
    int maxX = (this->BornX.Y - this->BornX.X) / this->CubeSize.X;
    int maxY = (this->BornY.Y - this->BornY.X) / this->CubeSize.Y;
    int maxZ = (this->BornZ.Y - this->BornZ.X) / this->CubeSize.Z;
    int size = maxX * maxY * maxZ;

    TArray<CoordInfo> list;
    list.Reserve(size);

    // creer un vector, pour chaque element, on calcul les index des précédents voisins qu'on a forcément déjà rencontré,
    // et on les link entre eux si cela remplis 
    // puis on refera un parcours à la fin pour séparer en différent mesh

    // create index
    auto lb_createIndex = [&maxY, &maxZ](int _x, int _y, int _z, int& _index) -> bool
    {
        auto lb_valid = [](int _value) -> bool
        {
            return _value >= 0;
        };

        if (lb_valid(_x) && lb_valid(_y) && lb_valid(_z))
        {
            _index = _z + _y * maxZ + _x * maxY * maxZ;
            return true;
        }
        return false;
    };

    // fill
    auto lb_neighboor = [&list, &lb_createIndex](int _x, int _y, int _z, CoordInfo& _info, int _currIdx, EFace _f1, EFace _f2)
    {
        int id = 0;
        if (lb_createIndex(_x, _y, _z, id))
        {
            CoordInfo& ref = list[id];
            if (ref.isValid())
            {
                ref.m_chainedLocation->addNeighbor(_f1, _info.m_chainedLocation);
                ref.m_neighboor[_f1] = _currIdx;

                _info.m_chainedLocation->addNeighbor(_f2, ref.m_chainedLocation);
                _info.m_neighboor[_f2] = id;
            }
        }
    };

    for (int x = 0; x < maxX; ++x)
    {
        for (int y = 0; y < maxY; ++y)
        {
            for (int z = 0; z < maxZ; ++z)
            {
                FVector location = FVector(x * this->CubeSize.X, y * this->CubeSize.Y, z * this->CubeSize.Z);
                float noise = getNoise(location);

                // create current object
                CoordInfo current = CoordInfo({ noise, false });
                if (current.isValid()) // if valide we will check neighboor
                {
                    current.m_chainedLocation = MakeShareable(new ChainedLocation(location, this->CubeSize));

                    int currIdx = 0;
                    lb_createIndex(x, y, z, currIdx); // it will be a valid index

                    // we have already estimate right, top, back
                    lb_neighboor(x - 1, y, z, current, currIdx, EFace::Right, EFace::Left);
                    lb_neighboor(x, y - 1, z, current, currIdx, EFace::Top, EFace::Bot);
                    lb_neighboor(x, y, z - 1, current, currIdx, EFace::Back, EFace::Front);
                }

                list.Add(current);
            }
        }
    }

    int id = 0;
    while (id < list.Num())
    {
        CoordInfo & info = list[id];
        if (info.isValid())
        {
            addNeighboor(info, list);
            // add component
            addProceduralMesh();

            // don't need to remove item in array, info.isValid() will be false if we already use item
            // and we don't remove it, because our system keep index !
        }
        ++id;
    }
}
/*
void AEnvironmentManager::createProceduralWorld()
{
	TArray<FVector> openList;

    //TMap<FVector, noise value and if we use it

	for (float x = 0; x < BornX.Y - BornX.X; x += CubeSize.X)
	{
		for (float y = 0; y < BornY.Y - BornY.X; y += CubeSize.Y)
		{
			for (float z = 0; z < BornZ.Y - BornZ.X; z += CubeSize.Z)
			{
                FVector location = FVector(x, y, z);
				if (isValidNoise(location))
				{
					openList.Add(location);
				}
			}
		}
	}

	while (openList.Num())
	{
		FVector location = openList[0];
		openList.RemoveAt(0);
		createChain(location, openList);
		
		// add component
		addProceduralMesh();
	}
}*/

TSharedPtr<ChainedLocation> AEnvironmentManager::createChain(FVector const& _location, TArray<FVector>& _openList)
{
	TSharedPtr<ChainedLocation> newPos = MakeShareable(new ChainedLocation(_location, CubeSize));

	m_currentObject.Add(newPos);

	addNeighboor(_openList, _location + FVector(0, CubeSize.Y, 0), EFace::Top, newPos, EFace::Bot);
	addNeighboor(_openList, _location + FVector(0, -CubeSize.Y, 0), EFace::Bot, newPos, EFace::Top);
	addNeighboor(_openList, _location + FVector(CubeSize.X, 0, 0), EFace::Right, newPos, EFace::Left);
	addNeighboor(_openList, _location + FVector(-CubeSize.X, 0, 0), EFace::Left, newPos, EFace::Right);
	addNeighboor(_openList, _location + FVector(0, 0, CubeSize.Z), EFace::Back, newPos, EFace::Front);
	addNeighboor(_openList, _location + FVector(0, 0, -CubeSize.Z), EFace::Front, newPos, EFace::Back);
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

void AEnvironmentManager::addNeighboor(CoordInfo& _info, TArray<CoordInfo> & _list)
{
    m_currentObject.Add(_info.m_chainedLocation);
    _info.m_use = true;

    auto lb_addNeighboor = [&](EFace _face)
    {
        int idx = _info.m_neighboor[_face];
        if (idx != -1 && !_list[idx].m_use)
        {
            addNeighboor(_list[idx], _list);
        }
    };

    lb_addNeighboor(EFace::Back);
    lb_addNeighboor(EFace::Front);
    lb_addNeighboor(EFace::Bot);
    lb_addNeighboor(EFace::Top);
    lb_addNeighboor(EFace::Right);
    lb_addNeighboor(EFace::Left);
}

void AEnvironmentManager::addProceduralMesh()
{
	UWorld* const world = GetWorld();
	if (world)
	{
		FVector location = FVector(BornX.X, BornY.X, BornZ.X);

		USpacelProceduralMeshComponent* proceduralMesh = NewObject<USpacelProceduralMeshComponent>(this);
		proceduralMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
		proceduralMesh->RegisterComponent();

		proceduralMesh->SetWorldLocation(location);
		proceduralMesh->bUseAsyncCooking = true;
		proceduralMesh->setCubeSize(CubeSize);
		proceduralMesh->setEdges(std::forward<TArray<TSharedPtr<ChainedLocation>>>(m_currentObject));
        proceduralMesh->SetCastShadow(false);
		m_currentObject.Empty();

		m_proceduralMeshComponents.Add(proceduralMesh);
	}
}

bool AEnvironmentManager::isValidNoise(FVector const& _location) const
{
    // increase float broke bloc, increase int (octave) add more bloc
    float noise = SpacelNoise::getInstance()->getOctaveNoise((_location.X + BornX.X) * 0.00007f, (_location.Y + BornY.X) * 0.00007f, (_location.Z + BornZ.X) * 0.00007f, 2);
    return noise >= .75f;
}

bool AEnvironmentManager::isValidLocation(FVector const& _location) const
{
	return (_location.X + BornX.X) >= BornX.X && (_location.X + BornX.X) < BornX.Y 
		&& (_location.Y + BornY.X) >= BornY.X && (_location.Y + BornY.X) < BornY.Y 
		&& (_location.Z + BornZ.X) >= BornZ.X && (_location.Z + BornZ.X) < BornZ.Y;
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

float AEnvironmentManager::getNoise(FVector const& _location) const
{
    // increase float broke bloc, increase int (octave) add more bloc
    return SpacelNoise::getInstance()->getOctaveNoise((_location.X + BornX.X) * 0.00007f, (_location.Y + BornY.X) * 0.00007f, (_location.Z + BornZ.X) * 0.00007f, 2);
}
