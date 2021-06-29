// Fill out your copyright notice in the Description page of Project Settings.


#include "LocalPlayerActionComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Camera/CameraComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShipPawn.h"
#include "Player/ModuleComponent.h"
#include "Player/Common/CommonPawn.h"
#include "World/MatiereManager.h"
#include "DataAsset/PlayerDataAsset.h"

ULocalPlayerActionComponent::ULocalPlayerActionComponent()
{
    // Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
    // off to improve performance if you don't need them.
    PrimaryComponentTick.bCanEverTick = true;
}

ULocalPlayerActionComponent::~ULocalPlayerActionComponent() = default;

void ULocalPlayerActionComponent::BeginPlay()
{
    Super::BeginPlay();

    m_metric = std::make_unique<LocalMetric>();

    if (ACommonPawn* pawn = get())
    {
        if (pawn->SpeedLinesComponent != nullptr)
        {
            m_speedLineMaterial = UMaterialInstanceDynamic::Create(pawn->MaterialSpeedLines, this);
            pawn->SpeedLinesComponent->AddOrUpdateBlendable(m_speedLineMaterial);
        }

        if (AShipPawn* shipPawn = get<AShipPawn>())
        {
            shipPawn->OnLocalTeamUpdateDelegate.AddDynamic(this, &ULocalPlayerActionComponent::OnUpdateTeam);

            shipPawn->OnAddEffectDelegate.AddDynamic(this, &ULocalPlayerActionComponent::AddEffect);
            shipPawn->OnRemoveEffectDelegate.AddDynamic(this, &ULocalPlayerActionComponent::RemoveEffect);
        }
    }
}

void ULocalPlayerActionComponent::TickComponent(float _deltaTime, ELevelTick _tickType, FActorComponentTickFunction* _thisTickFunction)
{
    Super::TickComponent(_deltaTime, _tickType, _thisTickFunction);

    ACommonPawn* pawn = get();
    if (m_speedLineMaterial != nullptr && pawn != nullptr && pawn->CameraComponent != nullptr)
    {
        float coefSpeed = FMath::Max((pawn->ModuleComponent->SupportMeshComponent->GetInstanceCount() / 9.0f), pawn->PlayerDataAsset->MinCoefSpeed);
        if (pawn->hasEffect(EEffect::MetaFormAttack) || pawn->hasEffect(EEffect::MetaFormProtection) || pawn->hasEffect(EEffect::MetaFormSupport) || pawn->hasEffect(EEffect::EscapeMode))
        {
            // override speed max
            coefSpeed = pawn->PlayerDataAsset->EscapeModeCoef;
        }
        // set material parameter
        float speedRef = FMath::Max(pawn->RU_PercentSpeed, pawn->R_OverDrive);
        float percent = FMath::Clamp(speedRef * coefSpeed, 0.0f, 2.6f);
        float multiplicator = pawn->hasEffect(EEffect::StartGame) ? 4.0f : 2.0f;
        m_speedLineMaterial->SetScalarParameterValue("Weight", percent * multiplicator);
        m_speedLineMaterial->SetVectorParameterValue("SpeedLinesColor", FMath::Lerp(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f), FLinearColor(0.0f, 0.943892f, 1.0f, 1.0f), percent));

        // set fov
        percent = FMath::Clamp(pawn->RU_PercentSpeed * coefSpeed, 0.0f, 2.6f);
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

void ULocalPlayerActionComponent::AddEffect(EEffect _effect)
{
    if (_effect == EEffect::StartGame)
    {
        if (ACommonPawn* pawn = get())
        {
            if (pawn->SpeedLinesComponent != nullptr && m_speedLineMaterial != nullptr)
            {
                m_speedLineMaterial->SetScalarParameterValue("GradientRadius", 0.93f);
            }
        }
    }
    else if (_effect == EEffect::MetaFormProtection)
    {
        if (ACommonPawn* pawn = get())
        {
            if (pawn->SpeedLinesComponent != nullptr && m_speedLineMaterial != nullptr)
            {
                m_speedLineMaterial->SetScalarParameterValue("Meta", 1.0f);
            }
        }
    }
}

void ULocalPlayerActionComponent::RemoveEffect(EEffect _effect)
{
    if (_effect == EEffect::StartGame)
    {
        if (ACommonPawn* pawn = get())
        {
            if (pawn->SpeedLinesComponent != nullptr && m_speedLineMaterial != nullptr)
            {
                m_speedLineMaterial->SetScalarParameterValue("GradientRadius", 1.0f);
            }
        }
    }
    else if (_effect == EEffect::MetaFormProtection)
    {
        if (ACommonPawn* pawn = get())
        {
            if (pawn->SpeedLinesComponent != nullptr && m_speedLineMaterial != nullptr)
            {
                m_speedLineMaterial->SetScalarParameterValue("Meta", 0.0f);
            }
        }
    }
}
