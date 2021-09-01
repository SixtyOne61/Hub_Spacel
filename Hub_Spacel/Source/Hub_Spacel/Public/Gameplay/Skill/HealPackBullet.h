// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Bullet/ProjectileBase.h"
#include "HealPackBullet.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API AHealPackBullet : public AProjectileBase
{
	GENERATED_BODY()
	
public:
	AHealPackBullet();

	UPROPERTY()
	uint8 Value {};

protected:
	UPROPERTY(EditAnywhere, Category = "Default")
	class UProjectileMovementComponent* ProjectileMovementComponent{ nullptr };
};
