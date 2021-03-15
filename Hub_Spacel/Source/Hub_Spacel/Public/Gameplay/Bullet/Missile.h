// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Bullet/ProjectileBase.h"
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
	void OnTargetFogIn();

protected:
	void BeginPlay() override;
	void Tick(float _deltaTime) override;
	bool OnHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit) override;

private:
	UFUNCTION()
	void OnRep_Cube();

	void updateLocations();

	UFUNCTION()
	void FlyToTarget();

	UFUNCTION()
	void Seek();

public:
	UPROPERTY(VisibleAnywhere, Category = "Default")
	class UInstancedStaticMeshComponent* Cubes { nullptr };

private:
	UPROPERTY()
	class AActor* Target { nullptr };

	UPROPERTY(ReplicatedUsing = "OnRep_Cube")
	TArray<FVector> RU_CubeLocations{};

	bool m_isSeekPlayer { false };
};
