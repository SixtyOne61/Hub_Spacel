// Fill out your copyright notice in the Description page of Project Settings.


#include "LocalPlayerActionComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Camera/CameraComponent.h"
#include "Components/PostProcessComponent.h"
#include "Mesh/SpacelInstancedMeshComponent.h"
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

            shipPawn->OnAddEffectClientDelegate.AddDynamic(this, &ULocalPlayerActionComponent::AddEffect);
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
        bool hasBoost = pawn->hasEffect(EEffect::EscapeMode) || pawn->hasEffect(EEffect::MetaFormAttack) || pawn->hasEffect(EEffect::MetaFormProtection) || pawn->hasEffect(EEffect::MetaFormSupport);
        float coefSpeed = FMath::Max((pawn->SupportComponent->GetNum() / 9.0f), pawn->PlayerDataAsset->MinCoefSpeed);
        if (hasBoost)
        {
            // override speed max
            coefSpeed = pawn->PlayerDataAsset->EscapeModeCoef;
        }

        // set material parameter
        float speedRef = FMath::Max(pawn->RU_PercentSpeed, (pawn->R_OverDrive / 10.0f));
        float percent = FMath::Clamp(speedRef * coefSpeed, 0.0f, 2.6f);
        float multiplicator = hasBoost ? 3.0f : 2.0f;

        // on missile effect we are stop but RU_PercentSpeed stay depend of Z
        if (pawn->hasEffect(EEffect::Missile))
        {
            percent = 0.0f;
        }

        m_speedLineMaterial->SetScalarParameterValue("Weight", percent * multiplicator);
        m_speedLineMaterial->SetVectorParameterValue("SpeedLinesColor", FMath::Lerp(FLinearColor(1.0f, 1.0f, 1.0f, 0.5f), FLinearColor(0.0f, 0.943892f, 1.0f, 0.8f), percent));

        // set fov
        float noSmoothFov { FMath::Lerp(90.0f, 130.0f, percent) };
        float currentFov { get()->CameraComponent->FieldOfView };
        float smoothFov { FMath::Lerp(currentFov, noSmoothFov, _deltaTime) };
        get()->CameraComponent->SetFieldOfView(smoothFov);
    }

    if (m_countDownRespawn > 0.0f)
    {
        CountDownRespawn(_deltaTime);
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
    else if (_effect == EEffect::Killed)
    {
        // TO DO : no magic number
        m_countDownRespawn = 10.0f;
    }
}

void ULocalPlayerActionComponent::CountDownRespawn(float _deltaSeconde)
{
    if (AShipPawn* pawn = get<AShipPawn>())
    {
        int round = m_countDownRespawn;
        m_countDownRespawn = FMath::Max(m_countDownRespawn - _deltaSeconde, 0.0f);
        if (round != (int)m_countDownRespawn)
        {
            pawn->OnSendInfoPlayerDelegate.Broadcast("Rebuilding... " + FString::FromInt((int)m_countDownRespawn));
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
    else if (_effect == EEffect::Killed)
    {
        m_countDownRespawn = 0.0f;
    }
}

