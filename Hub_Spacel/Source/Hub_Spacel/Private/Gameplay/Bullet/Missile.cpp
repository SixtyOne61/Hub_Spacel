// Fill out your copyright notice in the Description page of Project Settings.


#include "Missile.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Util/SimplyMath.h"
#include "GameFramework/ProjectileMovementComponent.h"

AMissile::AMissile()
    : AProjectileBase()
{
    Cubes = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("InstancedStaticMesh"));
    if (!ensure(Cubes != nullptr)) return;
    Cubes->SetupAttachment(RootComponent);

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

void AMissile::updateLocations()
{
    this->RU_CubeLocations.Empty();
    for (int32 i { 0 }; i < this->Cubes->GetInstanceCount(); ++i)
    {
        FTransform out {};
        this->Cubes->GetInstanceTransform(i, out, false);
        this->RU_CubeLocations.Add(out.GetLocation());
    }
}

void AMissile::applyHit(TArray<int32>& _instance)
{
    Super::applyHit(_instance);

    _instance.Sort([](int32 const& _a, int32 const& _b)
        {
            return _a > _b;
        });

    if (this->Cubes == nullptr) return;

    for (int32 id : _instance)
    {
        this->Cubes->RemoveInstance(id);
    }

    this->Cubes->GetInstanceCount() == 0 ? this->Destroy() : updateLocations();
}

void AMissile::OnRep_Cube()
{
    if(this->Cubes == nullptr) return;

    this->Cubes->ClearInstances();
    for (FVector const& location : RU_CubeLocations)
    {
        FTransform in {};
        in.SetLocation(location);
        this->Cubes->AddInstance(in);
    }
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

void AMissile::OnTargetFogIn()
{
    this->Destroy();
}


void AMissile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMissile, RU_CubeLocations);
}