// Fill out your copyright notice in the Description page of Project Settings.


#include "LaserBullet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Util/Tag.h"
#include "Player/ModuleComponent.h"
#include "DataAsset/TeamColorDataAsset.h"

ALaserBullet::ALaserBullet()
    : AProjectileBase()
{
    ProjectileCollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("ProjectileCollision"));
    RootComponent = ProjectileCollisionComponent;
}

void ALaserBullet::BeginPlay()
{
    Super::BeginPlay();

    if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        if (!ensure(ProjectileCollisionComponent != nullptr)) return;
        this->ProjectileCollisionComponent->OnComponentHit.AddDynamic(this, &ALaserBullet::OnComponentHit);
    }

    if (UStaticMeshComponent* comp = Cast<UStaticMeshComponent>(this->GetComponentByClass(UStaticMeshComponent::StaticClass())))
    {
        if (this->Colors != nullptr)
        {
            FString team = getLocalTeam();
            FColor color = this->Colors->GetColor<FColor>(team);
            comp->SetVectorParameterValueOnMaterials("Color", FVector(color.R / 255.0f, color.G / 255.0f, color.B / 255.0f));
        }
    }
}

void ALaserBullet::OnComponentHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
    this->OnHit(_hitComp, _otherActor, _otherComp, _normalImpulse, _hit);
}

void ALaserBullet::applyHit(TArray<int32>& _instance)
{
    Super::applyHit(_instance);
    this->Destroy();
}

bool ALaserBullet::OnHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
    if (Super::OnHit(_hitComp, _otherActor, _otherComp, _normalImpulse, _hit))
    {
        this->Destroy();
        return true;
    }

   return false;
}
