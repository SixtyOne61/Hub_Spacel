// Fill out your copyright notice in the Description page of Project Settings.


#include "HealPackBullet.h"
#include "GameFramework/ProjectileMovementComponent.h"

AHealPackBullet::AHealPackBullet()
{
    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
}