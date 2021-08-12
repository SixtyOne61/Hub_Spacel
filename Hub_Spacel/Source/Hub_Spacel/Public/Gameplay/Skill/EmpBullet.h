// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Bullet/ProjectileBase.h"
#include "EmpBullet.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API AEmpBullet : public AProjectileBase
{
	GENERATED_BODY()
	
public:
	AEmpBullet();

public:
	UPROPERTY()
	int EffectDuration { 0 };

protected:
	UPROPERTY(EditAnywhere, Category = "Default")
	class UProjectileMovementComponent* ProjectileMovementComponent{ nullptr };
};
