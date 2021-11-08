// Fill out your copyright notice in the Description page of Project Settings.


#include "EmpBullet.h"
#include "GameFramework/ProjectileMovementComponent.h"

AEmpBullet::AEmpBullet()
{
    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
}

