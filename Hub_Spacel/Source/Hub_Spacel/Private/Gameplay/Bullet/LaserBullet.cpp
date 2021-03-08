// Fill out your copyright notice in the Description page of Project Settings.


#include "LaserBullet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraFunctionLibrary.h"
#include "Util/Tag.h"
#include "Player/ModuleComponent.h"

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
