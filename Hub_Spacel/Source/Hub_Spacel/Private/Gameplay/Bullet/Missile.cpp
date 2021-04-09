// Fill out your copyright notice in the Description page of Project Settings.


#include "Missile.h"
#include "Net/UnrealNetwork.h"
#include "Util/SimplyMath.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"

AMissile::AMissile()
    : AProjectileBase()
{
    ProjectileCollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("ProjectileCollision"));
    RootComponent = ProjectileCollisionComponent;

    PrimaryActorTick.bCanEverTick = true;
}

AMissile::~AMissile()
{
}

void AMissile::BeginPlay()
{
    Super::BeginPlay();

    if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        m_isSeekPlayer = false;
        FTimerHandle handle;
        this->GetWorldTimerManager().SetTimer(handle, this, &AMissile::Seek, 0.2f, false);
        if (!ensure(ProjectileCollisionComponent != nullptr)) return;
        this->ProjectileCollisionComponent->OnComponentHit.AddDynamic(this, &AMissile::OnComponentHit);
    }
}

void AMissile::Seek()
{
    m_isSeekPlayer = true;
}

void AMissile::Tick(float _deltaTime)
{
    Super::Tick(_deltaTime);
    if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        if (m_isSeekPlayer)
        {
            FlyToTarget();
        }
    }
}

void AMissile::OnComponentHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
    this->OnHit(_hitComp, _otherActor, _otherComp, _normalImpulse, _hit);
}

bool AMissile::OnHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
    if (Super::OnHit(_hitComp, _otherActor, _otherComp, _normalImpulse, _hit))
    {
        TArray<int32> instance {_hit.Item};
        applyHit(instance);
        return true;
    }
    return false;
}

void AMissile::applyHit(TArray<int32>& _instance)
{
    Super::applyHit(_instance);
    this->Destroy();
}

void AMissile::FlyToTarget()
{
    if (this->Target == nullptr)
    {
        this->Destroy();
        return;
    }

    FVector dir = (this->Target->GetActorLocation() - this->GetActorLocation()).GetSafeNormal();
    this->SetActorRotation(dir.ToOrientationQuat());
    this->ProjectileMovementComponent->Velocity = dir * this->ProjectileMovementComponent->InitialSpeed;
}

void AMissile::OnTargetEffect(EEffect _type)
{
    if (_type == EEffect::Fog || _type == EEffect::MetaFormSupport)
    {
        this->Destroy();
    }
}
