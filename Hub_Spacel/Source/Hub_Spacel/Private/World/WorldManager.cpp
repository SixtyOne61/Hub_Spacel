// Fill out your copyright notice in the Description page of Project Settings.

#include "Hub_Spacel/Public/World/WorldManager.h"
#include "EnvironmentManager.h"
#include "DataAsset/WorldDataAsset.h"
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

    if (!ensure(this->WorldDataAsset != nullptr)) return;

    // define chunck size by number of chunk
    if (!ensure(this->BoxComponent != nullptr)) return;
    int chunckSize = this->WorldDataAsset->NbCubePerChunckPerAxis * this->WorldDataAsset->CubeSize;
    int environmentSize = this->WorldDataAsset->NbChunckPerAxis * this->WorldDataAsset->NbCubePerChunckPerAxis * this->WorldDataAsset->CubeSize;
    FVector box = FVector(environmentSize, environmentSize, environmentSize);
    this->BoxComponent->InitBoxExtent(box);

    FVector offset = box / 2.0f;

    int nb = this->WorldDataAsset->NbChunckPerAxis;
    int32 cubeSize = this->WorldDataAsset->CubeSize;

    for (int x = 0; x < nb; ++x)
    {
        for (int y = 0; y < nb; ++y)
        {
            for (int z = 0; z < nb; ++z)
            {
                // new location
                FVector location(x * chunckSize - offset.X, y * chunckSize - offset.Y, z * chunckSize - offset.Z);

                // add it to transform
                FTransform transform;
                transform.SetLocation(location);

                // TO DO : we can do better, put init in static function and check if we need to spawn an actor
                // start spawning actor
                AEnvironmentManager* environment = world->SpawnActorDeferred<AEnvironmentManager>(this->EnvironmentClass, transform);
                if (environment)
                {
                    // init component
                    bool isUsed = environment->init(FVector2D(location.X, location.X + chunckSize), FVector2D(location.Y, location.Y + chunckSize), FVector2D(location.Z, location.Z + chunckSize), FVector(cubeSize, cubeSize, cubeSize));
                    environment->FinishSpawning(transform);

                    if (isUsed == false)
                    {
                        environment->Destroy();
                    }
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


