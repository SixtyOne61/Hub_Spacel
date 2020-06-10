// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultSubMachine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"

// Sets default values
ADefaultSubMachine::ADefaultSubMachine()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    DefaultBulletSpawner = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DefaultBulletSpawner"));
    DefaultBulletSpawner->SetEnableGravity(false);
    DefaultBulletSpawner->SetGenerateOverlapEvents(false);
    DefaultBulletSpawner->SetCollisionProfileName("NoCollision");
    DefaultBulletSpawner->SetupAttachment(RootComponent);
    DefaultBulletSpawner->SetupAttachment(RootComponent);

    m_bulletSpawners.Add(DefaultBulletSpawner);
}

// Called when the game starts or when spawned
void ADefaultSubMachine::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        SetReplicates(true);
        SetReplicateMovement(true);
    }
}

// Called every frame
void ADefaultSubMachine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ADefaultSubMachine::getWoldLocationBulletSpawner(FVector& _out)
{
    if (this->m_idBulletSpawner >= this->m_bulletSpawners.Num())
    {
        // reset, but never happen
        this->m_idBulletSpawner = 0;
        return false;
    }

    auto lb_nextId = [&]()
    {
        ++this->m_idBulletSpawner;
        if (this->m_idBulletSpawner >= this->m_bulletSpawners.Num())
        {
            this->m_idBulletSpawner = 0;
        }
    };

    if (!this->m_bulletSpawners[this->m_idBulletSpawner])
    {
        lb_nextId();
        return false;
    }

    _out = this->m_bulletSpawners[this->m_idBulletSpawner]->GetComponentTransform().GetLocation();
    lb_nextId();
    return true;
}

