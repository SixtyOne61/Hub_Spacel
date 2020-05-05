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
    int max = list.Num();
    while (id < max)
    {
        CoordInfo & info = list[id];
        if (info.isValid())
        {
            // max item in next object
            m_currentObject.Reserve(max - id);

            addNeighboor(info, list);
            // add component
            addProceduralMesh();

            // don't need to remove item in array, info.isValid() will be false if we already use item
            // and we don't remove it, because our system keep index !
        }
        ++id;
    }
}

void AEnvironmentManager::addNeighboor(CoordInfo& _info, TArray<CoordInfo> & _list)
{
    m_currentObject.Add(_info.m_chainedLocation);
    _info.m_use = true;

    auto lb_addNeighboor = [&](EFace _face)
    {
        int idx = _info.m_neighboor[_face];
        if (idx != -1)
        {
            CoordInfo& next = _list[idx];
            if (!next.m_use)
            {
                addNeighboor(next, _list);
            }
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

float AEnvironmentManager::getNoise(FVector const& _location) const
{
    // increase float broke bloc, increase int (octave) add more bloc
    return SpacelNoise::getInstance()->getOctaveNoise((_location.X + BornX.X) * 0.00007f, (_location.Y + BornY.X) * 0.00007f, (_location.Z + BornZ.X) * 0.00007f, 2);
}