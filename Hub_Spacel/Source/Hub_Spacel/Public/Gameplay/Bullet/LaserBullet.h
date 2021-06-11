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

protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, Category = "Default")
	class UProjectileMovementComponent* ProjectileMovementComponent{ nullptr };

	UPROPERTY(EditAnywhere)
	class UTeamColorDataAsset* Colors{ nullptr };
};
