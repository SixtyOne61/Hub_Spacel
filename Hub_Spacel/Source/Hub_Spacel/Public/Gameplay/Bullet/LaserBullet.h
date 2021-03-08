// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Bullet/ProjectileBase.h"
#include "LaserBullet.generated.h"

UCLASS()
class HUB_SPACEL_API ALaserBullet : public AProjectileBase
{
	GENERATED_BODY()
	
public:
    /* override */
    void applyHit(TArray<int32>& _instance) override;

protected:
    bool OnHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit) override;
};
