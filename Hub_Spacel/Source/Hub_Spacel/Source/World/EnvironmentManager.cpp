// Fill out your copyright notice in the Description page of Project Settings.


#include "EnvironmentManager.h"
#include "Hub_Spacel/Source/Noise/SpacelNoise.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Hub_Spacel/Source/Mesh/SpacelProceduralMeshComponent.h"
#include "Hub_Spacel/Hub_SpacelGameInstance.h"
#include "XmlFile.h"
#include "XmlNode.h"
#include "Kismet/GameplayStatics.h"

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

void AEnvironmentManager::Init(FVector2D const& _bornX, FVector2D const& _bornY, FVector2D const& _bornZ, FVector const& _cubeSize)
{
	this->BornX = _bornX;
	this->BornY = _bornY;
	this->BornZ = _bornZ;
	this->CubeSize = _cubeSize;
}

// Called when the game starts or when spawned
void AEnvironmentManager::BeginPlay()
{
	Super::BeginPlay();

    if (this->HasAuthority())
    {
        this->SetReplicates(true);
    }

    UHub_SpacelGameInstance * spacelGameInstance = Cast<UHub_SpacelGameInstance>(this->GetGameInstance());
    if (!ensure(spacelGameInstance != nullptr)) return;

    // create xml or only read it
    if (spacelGameInstance->GenerateMap)
    {
        // create procedural world, and create xml
        generateEnvironment();
    }
    else
    {
        // create procedural world, but only read info from xml
        readXml();
    }
}

void AEnvironmentManager::generateEnvironment()
{
    int maxX = (this->BornX.Y - this->BornX.X) / this->CubeSize.X;
    int maxY = (this->BornY.Y - this->BornY.X) / this->CubeSize.Y;
    int maxZ = (this->BornZ.Y - this->BornZ.X) / this->CubeSize.Z;
    int size = maxX * maxY * maxZ;

    TArray<CoordInfo> list;
    list.Reserve(size);

    // create vector, for each element, we calculate last index of previoux neighboor,
    // we already meet them
    // then link between them if their match condition
    // at least, we will make a new loop for cut mesh

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
                    int currIdx = 0;
                    lb_createIndex(x, y, z, currIdx); // it will be a valid index

                    current.m_chainedLocation = MakeShareable(new ChainedLocation(location, this->CubeSize, currIdx));

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

    // check if we have object
    if (id < max)
    {
        // create xml file
        FXmlFile file;
        file.LoadFile("<root>\n</root>", EConstructMethod::ConstructFromBuffer);
        // root node
        FXmlNode * node = file.GetRootNode();
        if (!ensure(node != nullptr)) return;

        int idMesh = 0;
        while (id < max)
        {
            CoordInfo & info = list[id];
            if (info.isValid())
            {
                // max item in next object
                m_currentObject.Reserve(max - id);

                FString content = "\n";
                findMeshPoint(info, list, content);
                // add component
                createProceduralMeshComponent();

                node->AppendChildNode(FString::FromInt(idMesh), content);

                // don't need to remove item in array, info.isValid() will be false if we already use item
                // and we don't remove it, because our system keep index !
                ++idMesh;
            }
            ++id;
        }

        if (idMesh != 0)
        {
            bool isGold = UGameplayStatics::GetCurrentLevelName(this->GetWorld()).Contains("InGameLevel");
            FString path = FPaths::ProjectDir() + (isGold ? "Content/Xml/Gold/" : "Content/Xml/Test/") + this->GetActorLocation().ToString() + ".xml";
            file.Save(path);
        }
    }
}

void AEnvironmentManager::findMeshPoint(CoordInfo& _info, TArray<CoordInfo> & _list, FString & _xmlContent)
{
    m_currentObject.Add(_info.m_chainedLocation);
    _xmlContent.Append(_info.m_chainedLocation->getXml() + "\n");
    _info.m_use = true;

    auto lb_addNeighboor = [&](EFace _face)
    {
        int idx = _info.m_neighboor[_face];
        if (idx != -1)
        {
            CoordInfo& next = _list[idx];
            if (!next.m_use)
            {
                findMeshPoint(next, _list, _xmlContent);
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

bool AEnvironmentManager::readXml()
{
    bool isGold = UGameplayStatics::GetCurrentLevelName(this->GetWorld()).Contains("InGameLevel");
    FString path = FPaths::ProjectDir() + (isGold ? "Content/Xml/Gold/" : "Content/Xml/Test/") + this->GetActorLocation().ToString() + ".xml";

    FXmlFile file;
    if (!file.LoadFile(path))
    {
        return false;
    }

    FXmlNode * rootNode = file.GetRootNode();
    if (rootNode == nullptr)
    {
        return false;
    }

    TArray<FXmlNode*> const& childrenMesh = rootNode->GetChildrenNodes();
    for (auto * nodeMesh : childrenMesh)
    {
        // node contain all information for one mesh
        if (nodeMesh == nullptr)
        {
            continue;
        }

        TArray<FXmlNode*> const& childrenLocation = nodeMesh->GetChildrenNodes();
        for (auto * nodeLocation : childrenLocation)
        {
            if (nodeLocation == nullptr)
            {
                continue;
            }

            int id = FCString::Atoi(*(nodeLocation->GetAttribute("id")));
            FVector center = FVector::ZeroVector, size = FVector::ZeroVector;
            center.InitFromString(nodeLocation->GetAttribute("center"));
            size.InitFromString(nodeLocation->GetAttribute("size"));

            // TO DO : add neighboor
            m_currentObject.Add(MakeShareable(new ChainedLocation(center, size, id)));
        }

        createProceduralMeshComponent();
    }

    return true;
}

void AEnvironmentManager::createProceduralMeshComponent()
{
    FVector location = FVector(this->BornX.X, this->BornY.X, this->BornZ.X);

    USpacelProceduralMeshComponent* proceduralMesh = NewObject<USpacelProceduralMeshComponent>(this);
    if (!ensure(proceduralMesh != nullptr)) return;
    proceduralMesh->AttachToComponent(this->RootComponent, FAttachmentTransformRules::KeepWorldTransform);
    proceduralMesh->RegisterComponent();

    proceduralMesh->SetWorldLocation(location);
    proceduralMesh->bUseAsyncCooking = true;
    proceduralMesh->setCubeSize(this->CubeSize);
    proceduralMesh->setEdges(std::forward<TArray<TSharedPtr<ChainedLocation>>>(m_currentObject));
    proceduralMesh->SetCastShadow(false);
    proceduralMesh->setOwnerLocation(this->GetActorLocation());
    proceduralMesh->generateMesh(std::move(FName("BlockAll")));

    if (UMaterialInstanceDynamic * customMat = UMaterialInstanceDynamic::Create(this->MatAsteroid, proceduralMesh))
    {
        customMat->SetScalarParameterValue(TEXT("Trickness"), this->TricknessValue);
        proceduralMesh->SetMaterial(0, customMat);
        proceduralMesh->SetRenderCustomDepth(true);
        proceduralMesh->SetCustomDepthStencilValue(this->StencilValue);
    }

    // reset current object
    m_currentObject.Empty();

    // add to list
    this->ProceduralMeshComponents.Add(proceduralMesh);
}