// Fill out your copyright notice in the Description page of Project Settings.

#include "World/WorldManager.h"
#include "World/Chunck.h"
#include "DataAsset/WorldDataAsset.h"
#include "Components/BoxComponent.h"
#include "Mesh/SpacelProceduralMeshComponent.h"
#include "Player/ShipPawn.h"
#include "Util/SimplyMath.h"
#include "Util/Tag.h"

// Sets default values
AWorldManager::AWorldManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("Delimiter"));
    if (!ensure(BoxComponent != nullptr)) return;
    RootComponent = BoxComponent;

    Tags.Add(Tags::WorldManager);
}

// Called when the game starts or when spawned
void AWorldManager::BeginPlay()
{
	Super::BeginPlay();

    if (this->GetNetMode() != ENetMode::NM_DedicatedServer)
    {
        return;
    }

    UWorld* const world { this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    if (!ensure(this->WorldDataAsset != nullptr)) return;

    // define chunck size by number of chunk
    if (!ensure(this->BoxComponent != nullptr)) return;
    int chunckSize = this->WorldDataAsset->NbCubePerChunckPerAxis * this->WorldDataAsset->CubeSize;
    int environmentSize = this->WorldDataAsset->NbChunckPerAxis * this->WorldDataAsset->NbCubePerChunckPerAxis * this->WorldDataAsset->CubeSize;
    FVector box = FVector(environmentSize, environmentSize, environmentSize);
    this->BoxComponent->InitBoxExtent(box);

    this->BoxComponent->OnComponentEndOverlap.AddDynamic(this, &AWorldManager::OnComponentEndOverlap);

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
                AChunck* chunck = world->SpawnActorDeferred<AChunck>(this->ChunckClass, transform);
                if (chunck)
                {
                    // init component
                    chunck->init(chunckSize, cubeSize);
                    chunck->FinishSpawning(transform);
                }
            }
        }
    }
}

void AWorldManager::OnComponentEndOverlap(UPrimitiveComponent* _overlappedComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, int32 _otherBodyIndex)
{
    if (AShipPawn* pawn = Cast<AShipPawn>(_otherActor))
    {
        if (!pawn->hasEffect(EEffect::Killed))
        {
            FRotator rot = SimplyMath::MyLookRotation(this->GetActorLocation(), pawn->GetActorUpVector(), pawn->GetActorLocation());
            pawn->SetActorRotation(rot, ETeleportType::ResetPhysics);
            pawn->emp(1);
            UE_LOG(LogTemp, Warning, TEXT("Move pawn to zero"));
        }
    }
}
