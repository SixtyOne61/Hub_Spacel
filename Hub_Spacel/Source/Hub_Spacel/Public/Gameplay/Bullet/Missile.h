// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Bullet/ProjectileBase.h"
#include "Enum/SpacelEnum.h"
#include "Missile.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API AMissile : public AProjectileBase
{
	GENERATED_BODY()
	
	friend class UFireComponent;

public:
	AMissile();
	virtual ~AMissile();

	/* override */
	void applyHit(TArray<int32>& _instance) override;

	UFUNCTION()
	void OnTargetEffect(EEffect _type);

protected:
	void BeginPlay() override;
	void Tick(float _deltaTime) override;
	bool OnHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit) override;

	UFUNCTION()
	void OnComponentHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit);

private:
	UFUNCTION()
	void FlyToTarget();

	UFUNCTION()
	void Seek();

protected:
	UPROPERTY(EditAnywhere, Category = "Collision")
	class USphereComponent* ProjectileCollisionComponent{ nullptr };

private:
	UPROPERTY()
	class AActor* Target { nullptr };

	bool m_isSeekPlayer { false };
};
