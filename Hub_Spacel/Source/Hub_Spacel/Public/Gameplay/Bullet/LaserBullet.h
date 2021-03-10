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
	ALaserBullet();

    /* override */
    void applyHit(TArray<int32>& _instance) override;

protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;

    bool OnHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit) override;

private:
	UFUNCTION()
	void OnComponentHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit);

protected:
	UPROPERTY(EditAnywhere, Category = "Collision")
	class USphereComponent* ProjectileCollisionComponent{ nullptr };

	UPROPERTY(EditAnywhere)
	class UTeamColorDataAsset* Colors{ nullptr };
};
