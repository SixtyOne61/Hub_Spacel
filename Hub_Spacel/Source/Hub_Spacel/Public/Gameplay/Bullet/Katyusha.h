// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Bullet/ProjectileBase.h"
#include "katyusha.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API AKatyusha : public AProjectileBase
{
	GENERATED_BODY()
	
	friend class UFireComponent;

public:
	AKatyusha();
	virtual ~AKatyusha();

protected:
	void BeginPlay() override;
	void Tick(float _deltaTime) override;

	UFUNCTION(UnReliable, NetMulticast)
	void RPCNetMulticastSync(int64 _syncPoint, FVector const& _location);

private:
	UFUNCTION()
	void Seek();

protected:
	UPROPERTY(EditAnywhere)
	class UHomingMissileDataAsset* DataAsset{ nullptr };

	UPROPERTY(EditAnywhere, Category = "Setup")
	FVector TargetLocation {};

	UPROPERTY(Replicated)
	bool R_IsSeek{ false };
};
