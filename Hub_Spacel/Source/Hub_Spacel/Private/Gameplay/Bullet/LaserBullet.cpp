// Fill out your copyright notice in the Description page of Project Settings.


#include "LaserBullet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Util/Tag.h"
#include "Player/ModuleComponent.h"
#include "DataAsset/TeamColorDataAsset.h"

ALaserBullet::ALaserBullet()
    : AProjectileBase()
{
    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
}

void ALaserBullet::BeginPlay()
{
    Super::BeginPlay();

    if (UStaticMeshComponent* comp = Cast<UStaticMeshComponent>(this->GetComponentByClass(UStaticMeshComponent::StaticClass())))
    {
        if (this->Colors != nullptr)
        {
            FString team = getLocalTeam();
            FColor color = this->Colors->GetColor<FColor>(team);
            comp->SetVectorParameterValueOnMaterials("Color", FVector(color.R / 255.0f, color.G / 255.0f, color.B / 255.0f));
        }
    }
}

