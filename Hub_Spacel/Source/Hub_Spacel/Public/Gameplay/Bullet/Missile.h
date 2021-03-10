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

protected:
	void BeginPlay() override;
	bool OnHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit) override;

private:
	UFUNCTION()
	void OnRep_Cube();

	void updateLocations();

	UFUNCTION()
	void FlyToTarget();

public:
	UPROPERTY(VisibleAnywhere, Category = "Default")
	class UInstancedStaticMeshComponent* Cubes { nullptr };

private:
	UPROPERTY()
	FTimerHandle TimerHandle {};

	UPROPERTY()
	class AActor* Target { nullptr };

	UPROPERTY(ReplicatedUsing = "OnRep_Cube")
	TArray<FVector> RU_CubeLocations{};
};
