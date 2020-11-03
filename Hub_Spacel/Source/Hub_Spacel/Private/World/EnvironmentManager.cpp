// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvironmentManager.h"
#include "Noise/SpacelNoise.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Mesh/SpacelProceduralMeshComponent.h"
#include "Mesh/LocationInformation.h"
#include "Hub_SpacelGameInstance.h"

// Sets default values
AEnvironmentManager::AEnvironmentManager()
	: BornX()
	, BornY()
	, BornZ()
	, CubeSize(FVector::ZeroVector)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

bool AEnvironmentManager::init(FVector2D const& _bornX, FVector2D const& _bornY, FVector2D const& _bornZ, FVector const& _cubeSize)
{
	this->BornX = _bornX;
	this->BornY = _bornY;
	this->BornZ = _bornZ;
	this->CubeSize = _cubeSize;

    return generateEnvironment();
}

// Called when the game starts or when spawned
void AEnvironmentManager::BeginPlay()
{
	Super::BeginPlay();

    if (this->HasAuthority())
    {
        this->SetReplicates(true);
    }
}

bool AEnvironmentManager::generateEnvironment()
{
    int maxX = (this->BornX.Y - this->BornX.X) / this->CubeSize.X;
    int maxY = (this->BornY.Y - this->BornY.X) / this->CubeSize.Y;
    int maxZ = (this->BornZ.Y - this->BornZ.X) / this->CubeSize.Z;
    int size = maxX * maxY * maxZ;

    TArray<FLocationInformation> list;
    list.Reserve(size);
    list.SetNum(size);

    // create vector, for each element, we calculate last index of previoux neighboor,
    // we already meet them
    // then link between them if their match condition
    // at least, we will make a new loop for cut mesh

    // create index
    auto lb_createIndex = [&maxY, &maxZ](int _x, int _y, int _z, int& _index) -> bool
    {
        if (_x >= 0 && _y >= 0 && _z >= 0)
        {
            _index = _z + _y * maxZ + _x * maxY * maxZ;
            return true;
        }
        return false;
    };

    // fill
    auto lb_neighboor = [&list, &lb_createIndex](int _x, int _y, int _z, FLocationInformation& _info, EFace _f1, EFace _f2)
    {
        int id = 0;
        if (lb_createIndex(_x, _y, _z, id))
        {
            FLocationInformation& ref = list[id];
            if (ref.isValid())
            {
                ref.m_neighboor[_f1] = &_info;
                ref.setNeighboorId(_f1, _info.Index);
                ref.Mask |= _f1;

                _info.m_neighboor[_f2] = &ref;
                _info.setNeighboorId(_f2, ref.Index);
                _info.Mask |= _f2;
            }
        }
    };

    for (int x = 0; x < maxX; ++x)
    {
        for (int y = 0; y < maxY; ++y)
        {
            for (int z = 0; z < maxZ; ++z)
            {
                FVector location { x * this->CubeSize.X, y * this->CubeSize.Y, z * this->CubeSize.Z };
                float noise = getNoise(location);

                FLocationInformation currentNode { noise, false };

                // check if we have a valid noise
                if (currentNode.isValid())
                {
                    // create index of current Node
                    int currentIndex = 0;
                    // we are valid, so we can have a valid index
                    lb_createIndex(x, y, z, currentIndex);
                    currentNode.Index = currentIndex;
                    currentNode.Location = location;

                    // we have already estimate right, top, back
                    lb_neighboor(x - 1, y, z, currentNode, EFace::Right, EFace::Left);
                    lb_neighboor(x, y - 1, z, currentNode, EFace::Top, EFace::Bot);
                    lb_neighboor(x, y, z - 1, currentNode, EFace::Back, EFace::Front);
                    
                    list[currentIndex] = std::move(currentNode);
                }
            }
        }
    }

    int id = 0;
    int max = list.Num();

    // check if we have object
    if (id >= max)
    {
        return false;
    }

    while (id < max)
    {
        FLocationInformation & startComponent = list[id];
        if (startComponent.isValid())
        {
            TArray<FLocationInformation> nextObject;
            // max item in next object
            nextObject.Reserve(size);
            nextObject.SetNum(size);

            int nbPoint = 0;
            findMeshPoint(startComponent, nextObject, list, nbPoint);
            // add component
            createProceduralMeshComponent(std::move(nextObject), nbPoint);

            // don't need to remove item in array, info.isValid() will be false if we already use item
            // and we don't remove it, because our system keep index !
        }
        ++id;
    }

    return ProceduralMeshComponents.Num() != 0;
}

void AEnvironmentManager::findMeshPoint(FLocationInformation& _node, TArray<FLocationInformation> & _currentObject, TArray<FLocationInformation> & _list, int & _nbPoint)
{
    if (_node.Index == -1)
    {
        return;
    }

    _node.Used = true;
    _currentObject[_node.Index] = _node;
    _nbPoint++;

    auto lb_addNeighboor = [&](EFace _face)
    {
        int const& id = _node.getNeighboorId(_face);
        if (id != -1)
        {
            FLocationInformation & nextNode = _list[id];
            if (!nextNode.Used)
            {
                findMeshPoint(nextNode, _currentObject, _list, _nbPoint);
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

float AEnvironmentManager::getNoise(FVector const& _location) const
{
    // increase float broke bloc, increase int (octave) add more bloc
    return SpacelNoise::getInstance()->getOctaveNoise((_location.X + this->BornX.X) * 0.00007f, (_location.Y + this->BornY.X) * 0.00007f, (_location.Z + this->BornZ.X) * 0.00007f, 2);
}

void AEnvironmentManager::createProceduralMeshComponent(TArray<FLocationInformation> && _locations, int const& _nbPoint)
{
    FVector location = FVector(this->BornX.X, this->BornY.X, this->BornZ.X);

    USpacelProceduralMeshComponent* proceduralMesh = NewObject<USpacelProceduralMeshComponent>(this);
    if (!ensure(proceduralMesh != nullptr)) return;
    proceduralMesh->AttachToComponent(this->RootComponent, FAttachmentTransformRules::KeepWorldTransform);
    proceduralMesh->RegisterComponent();

    proceduralMesh->SetWorldLocation(location);
    proceduralMesh->bUseAsyncCooking = true;
    proceduralMesh->setCubeSize(this->CubeSize);
    proceduralMesh->setEdges(std::move(_locations));
    proceduralMesh->SetCastShadow(false);
    proceduralMesh->setOwnerLocation(this->GetActorLocation());
    proceduralMesh->generateMesh(std::move(FName("BlockAll")), _nbPoint);

    //customMat->SetScalarParameterValue(TEXT("Trickness"), this->TricknessValue);
    proceduralMesh->SetMaterial(0, this->MatAsteroid);
    proceduralMesh->SetRenderCustomDepth(true);
    proceduralMesh->SetCustomDepthStencilValue(this->StencilValue);

    // add to list
    this->ProceduralMeshComponents.Add(proceduralMesh);
}