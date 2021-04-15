// Fill out your copyright notice in the Description page of Project Settings.


#include "LocalPlayerActionComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Camera/CameraComponent.h"
#include "Components/PostProcessComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShipPawn.h"
#include "World/MatiereManager.h"

ULocalPlayerActionComponent::ULocalPlayerActionComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;
}

void ULocalPlayerActionComponent::BeginPlay()
{
    Super::BeginPlay();

    if (get() && get()->SpeedLinesComponent != nullptr)
    {
        m_postProcessMaterial = UMaterialInstanceDynamic::Create(get()->MaterialSpeedLines, this);
        get()->SpeedLinesComponent->AddOrUpdateBlendable(m_postProcessMaterial);

        if (get<AShipPawn>() != nullptr)
        {
            get<AShipPawn>()->OnLocalTeamUpdateDelegate.AddDynamic(this, &ULocalPlayerActionComponent::OnUpdateTeam);
        }
    }
}

void ULocalPlayerActionComponent::TickComponent(float _deltaTime, ELevelTick _tickType, FActorComponentTickFunction* _thisTickFunction)
{
    Super::TickComponent(_deltaTime, _tickType, _thisTickFunction);

    if (m_postProcessMaterial != nullptr && get() && get()->CameraComponent != nullptr)
    {
        // set material parameter
        float percent = FMath::Clamp(get()->RU_PercentSpeed, 0.0f, 1.0f);
        m_postProcessMaterial->SetScalarParameterValue("Weight", percent);
        m_postProcessMaterial->SetVectorParameterValue("SpeedLinesColor", FMath::Lerp(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f), FLinearColor(0.0f, 0.943892f, 1.0f, 1.0f), percent));

        // set fov
        float noSmoothFov { FMath::Lerp(90.0f, 130.0f, percent) };
        float currentFov { get()->CameraComponent->FieldOfView };
        float smoothFov { FMath::Lerp(currentFov, noSmoothFov, _deltaTime) };
        get()->CameraComponent->SetFieldOfView(smoothFov);
    }
}

void ULocalPlayerActionComponent::OnUpdateTeam(FString const& _team)
{
    if (AMatiereManager* matiereManager = Cast<AMatiereManager>(UGameplayStatics::GetActorOfClass(this->GetWorld(), AMatiereManager::StaticClass())))
    {
        matiereManager->LocalTeam = _team;
    }
}