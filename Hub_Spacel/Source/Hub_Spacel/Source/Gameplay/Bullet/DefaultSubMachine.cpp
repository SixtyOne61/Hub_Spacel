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
    if (!ensure(Root != nullptr)) return;
    RootComponent = Root;

    DefaultBulletSpawner = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DefaultBulletSpawner"));
    if (!ensure(DefaultBulletSpawner != nullptr)) return;
    DefaultBulletSpawner->SetEnableGravity(false);
    DefaultBulletSpawner->SetGenerateOverlapEvents(false);
    DefaultBulletSpawner->SetCollisionProfileName("NoCollision");
    DefaultBulletSpawner->SetupAttachment(RootComponent);

    m_bulletSpawners.Add(DefaultBulletSpawner);
}

// Called when the game starts or when spawned
void ADefaultSubMachine::BeginPlay()
{
    Super::BeginPlay();

    if (this->HasAuthority())
    {
        this->SetReplicates(true);
        this->SetReplicateMovement(true);
    }
}

// Called every frame
void ADefaultSubMachine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

bool ADefaultSubMachine::getWoldLocationBulletSpawner(FVector& _out)
{
    if (m_idBulletSpawner >= m_bulletSpawners.Num())
    {
        // reset, but never happen
        m_idBulletSpawner = 0;
        return false;
    }

    auto lb_nextId = [&]()
    {
        ++m_idBulletSpawner;
        if (m_idBulletSpawner >= m_bulletSpawners.Num())
        {
            m_idBulletSpawner = 0;
        }
    };

    if (!m_bulletSpawners[m_idBulletSpawner])
    {
        lb_nextId();
        return false;
    }

    _out = m_bulletSpawners[m_idBulletSpawner]->GetComponentTransform().GetLocation();
    lb_nextId();
    return true;
}

