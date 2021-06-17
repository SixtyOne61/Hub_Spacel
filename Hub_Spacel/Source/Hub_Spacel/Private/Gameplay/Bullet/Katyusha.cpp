// Fill out your copyright notice in the Description page of Project Settings.


#include "katyusha.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "DataAsset/HomingMissileDataAsset.h"

AKatyusha::AKatyusha()
    : AProjectileBase()
{
    PrimaryActorTick.bCanEverTick = true;
}

AKatyusha::~AKatyusha()
{
}

void AKatyusha::BeginPlay()
{
    Super::BeginPlay();

    if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        if (this->DataAsset == nullptr) return;

        this->R_IsSeek = false;
        FTimerHandle handle;
        this->GetWorldTimerManager().SetTimer(handle, this, &AKatyusha::Seek, this->DataAsset->TimeBeforeLock, false);
    }
}

void AKatyusha::Tick(float _deltaTime)
{
    Super::Tick(_deltaTime);

    if (this->DataAsset == nullptr) return;

    FVector dir = this->GetActorForwardVector();
    FVector const& actorLocation = this->GetActorLocation();
    float speed = this->DataAsset->SpeedPreLock;
    if (this->R_IsSeek)
    {
        dir = (this->R_TargetLocation - actorLocation).GetSafeNormal();
        speed = this->DataAsset->SpeedAfterLock;
    }

    FVector nextLocation = actorLocation + dir * speed * _deltaTime;
    this->SetActorLocation(nextLocation);

    if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        int64 syncTime = FDateTime::Now().ToUnixTimestamp();
        RPCNetMulticastSync(syncTime, this->GetActorLocation());
    }
}

void AKatyusha::Seek()
{
    this->R_IsSeek = true;
}

void AKatyusha::RPCNetMulticastSync_Implementation(int64 _syncPoint, FVector_NetQuantize const& _location)
{
    if (this->DataAsset == nullptr) return;

    int64 time = FDateTime::Now().ToUnixTimestamp();
    // readjust location with sync time
    int64 deltaTime = time - _syncPoint;

    FVector dir = this->GetActorForwardVector();
    float speed = this->DataAsset->SpeedPreLock;
    if (this->R_IsSeek)
    {
        dir = (this->R_TargetLocation - _location).GetSafeNormal();
        speed = this->DataAsset->SpeedAfterLock;
    }

    FVector nextLocation = _location + dir * speed * deltaTime;
    this->SetActorLocation(nextLocation);
}

void AKatyusha::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AKatyusha, R_TargetLocation);
    DOREPLIFETIME(AKatyusha, R_IsSeek);
}

