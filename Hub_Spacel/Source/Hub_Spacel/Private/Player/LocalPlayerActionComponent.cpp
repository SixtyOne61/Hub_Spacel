// Fill out your copyright notice in the Description page of Project Settings.


#include "LocalPlayerActionComponent.h"
#include "Materials/Material.h"
#include "Components/PostProcessComponent.h"
#include "Camera/CameraComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Player/ShipPawn.h"

ULocalPlayerActionComponent::ULocalPlayerActionComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;
}

void ULocalPlayerActionComponent::BeginPlay()
{
    Super::BeginPlay();

    if (m_shipPawnOwner.Get() && m_shipPawnOwner.Get()->SpeedLinesComponent != nullptr)
    {
        m_postProcessMaterial = UMaterialInstanceDynamic::Create(m_shipPawnOwner.Get()->MaterialSpeedLines, this);
        m_shipPawnOwner.Get()->SpeedLinesComponent->AddOrUpdateBlendable(m_postProcessMaterial);
    }
}

void ULocalPlayerActionComponent::TickComponent(float _deltaTime, ELevelTick _tickType, FActorComponentTickFunction* _thisTickFunction)
{
    Super::TickComponent(_deltaTime, _tickType, _thisTickFunction);

    if (m_postProcessMaterial != nullptr && m_shipPawnOwner.Get() && m_shipPawnOwner.Get()->CameraComponent != nullptr)
    {
        // set material parameter
        float percent = FMath::Clamp(m_shipPawnOwner.Get()->RU_PercentSpeed, 0.0f, 1.0f);
        m_postProcessMaterial->SetScalarParameterValue("Weight", percent);
        m_postProcessMaterial->SetVectorParameterValue("SpeedLinesColor", FMath::Lerp(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f), FLinearColor(0.0f, 0.943892f, 1.0f, 1.0f), percent));

        // set fov
        float noSmoothFov { FMath::Lerp(90.0f, 130.0f, percent) };
        float currentFov { m_shipPawnOwner.Get()->CameraComponent->FieldOfView };
        float smoothFov { FMath::Lerp(currentFov, noSmoothFov, _deltaTime) };
        m_shipPawnOwner.Get()->CameraComponent->SetFieldOfView(smoothFov);
    }
}