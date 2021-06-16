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

	UFUNCTION()
	void OnTargetEffect(EEffect _type);

	UFUNCTION(Reliable, NetMulticast)
	void RPCNetMulticastTarget(FName const& _targetName);

protected:
	void BeginPlay() override;
	void Tick(float _deltaTime) override;

private:
	UFUNCTION()
	void Seek();

private:
	UPROPERTY(EditAnywhere)
	class UHomingMissileDataAsset* DataAsset { nullptr };

	UPROPERTY()
	class AActor* Target { nullptr };

	UPROPERTY(Replicated)
	bool R_IsSeekPlayer { false };
};
