// Fill out your copyright notice in the Description page of Project Settings.


#include "LaserBullet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Util/Tag.h"
#include "Player/ModuleComponent.h"
#include "Player/ShipPawn.h"
#include "Player/LocalPlayerActionComponent.h"
#include "DataAsset/TeamColorDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Enum/SpacelEnum.h"

ALaserBullet::ALaserBullet()
    : AProjectileBase()
{
    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
}

void ALaserBullet::BeginPlay()
{
    Super::BeginPlay();

    m_startUnixTime = FDateTime::Now().ToUnixTimestamp();

    if (UStaticMeshComponent* comp = Cast<UStaticMeshComponent>(this->GetComponentByClass(UStaticMeshComponent::StaticClass())))
    {
        if (this->Colors != nullptr)
        {
            FString team = getLocalTeam();
            FColor color = this->Colors->GetColor<FColor>(team);
            comp->SetVectorParameterValueOnMaterials("Color", FVector(color.R / 255.0f, color.G / 255.0f, color.B / 255.0f));
        }
    }

    if (UNiagaraComponent* trail = Cast<UNiagaraComponent>(this->GetComponentByClass(UNiagaraComponent::StaticClass())))
    {
        if (this->Colors != nullptr)
        {
            FString team = getLocalTeam();
            FLinearColor color = this->Colors->GetColor<FColor>(team);

            trail->SetNiagaraVariableLinearColor("User.TrailColor", color);
        }
    }
}

void ALaserBullet::Destroyed()
{
    if (this->GetNetMode() != ENetMode::NM_DedicatedServer)
    {
        if (APlayerController* controller = UGameplayStatics::GetPlayerController(this->GetWorld(), 0))
        {
            if (AShipPawn* pawn = Cast<AShipPawn>(controller->GetPawn()))
            {
                if (ULocalPlayerActionComponent* localComponent = Cast<ULocalPlayerActionComponent>(pawn->GetComponentByClass(ULocalPlayerActionComponent::StaticClass())))
                {
                    int64 time = FDateTime::Now().ToUnixTimestamp();
                    bool isExpired = FMath::IsNearlyEqual(FMath::Abs(time - m_startUnixTime), this->InitialLifeSpan, 0.3f);
                    localComponent->createPrecisionData(isExpired);
                }
            }
        }
    }

    Super::Destroyed();
}

