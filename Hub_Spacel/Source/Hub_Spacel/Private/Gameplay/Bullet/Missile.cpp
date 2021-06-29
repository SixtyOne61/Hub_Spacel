// Fill out your copyright notice in the Description page of Project Settings.


#include "Missile.h"
#include "Net/UnrealNetwork.h"
#include "Util/SimplyMath.h"
#include "Util/Tag.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Misc/DateTime.h"
#include "Kismet/GameplayStatics.h"
#include "DataAsset/HomingMissileDataAsset.h"

AMissile::AMissile()
    : AProjectileBase()
{
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
        if (this->DataAsset == nullptr) return;

        this->R_IsSeekPlayer = false;
        FTimerHandle handle;
        this->GetWorldTimerManager().SetTimer(handle, this, &AMissile::Seek, this->DataAsset->TimeBeforeLock, false);
    }
}

void AMissile::Seek()
{
    this->R_IsSeekPlayer = true;
}

void AMissile::Tick(float _deltaTime)
{
    Super::Tick(_deltaTime);

    if(this->DataAsset == nullptr) return;

    if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        if (this->R_IsSeekPlayer)
        {
            if (this->R_Target == nullptr)
            {
                this->Destroy();
            }
        }
    }

    if (this->R_Target != nullptr)
    {
        FVector dir = this->GetActorForwardVector();
        FVector const& actorLocation = this->GetActorLocation();
        float speed = this->DataAsset->SpeedPreLock;
        if (this->R_IsSeekPlayer)
        {
            FVector const& targetLocation = this->R_Target->GetActorLocation();
            dir = (targetLocation - actorLocation).GetSafeNormal();
            speed = this->DataAsset->SpeedAfterLock;
        }

        FVector const& currentLocation = actorLocation;
        FVector nextLocation = currentLocation + dir * speed * _deltaTime;
        this->SetActorLocation(nextLocation);
    }
}

void AMissile::OnTargetEffect(EEffect _type)
{
    if (_type == EEffect::Fog || _type == EEffect::MetaFormSupport || _type == EEffect::Killed)
    {
        this->Destroy();
    }
}

void AMissile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AMissile, R_Target);
    DOREPLIFETIME(AMissile, R_IsSeekPlayer);
}
