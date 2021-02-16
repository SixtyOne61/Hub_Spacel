// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/DestroyActor.h"
#include "LaserBullet.generated.h"

UCLASS()
class HUB_SPACEL_API ALaserBullet : public ADestroyActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALaserBullet();

    /* override */
    void applyHit(TArray<int32>& _instance) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    UFUNCTION()
    void InitProfileCollision();

private:
    UFUNCTION()
    void OnComponentHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit);

public:
    UPROPERTY(EditAnywhere, Category = "Default")
    class UProjectileMovementComponent* ProjectileMovementComponent { nullptr };

    UPROPERTY(EditAnywhere, Category = "Collision")
    class USphereComponent* ProjectileCollisionComponent { nullptr };

protected:
    UPROPERTY(EditAnywhere)
    class UNiagaraSystem* FireFx{ nullptr };
};
