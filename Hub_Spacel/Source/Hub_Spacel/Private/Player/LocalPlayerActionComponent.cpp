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

    ACommonPawn* pawn = get();
    if (m_postProcessMaterial != nullptr && pawn != nullptr && pawn->CameraComponent != nullptr)
    {
        float coefSpeed = FMath::Max((pawn->ModuleComponent->SupportMeshComponent->GetInstanceCount() / 9.0f), pawn->PlayerDataAsset->MinCoefSpeed);
        if (pawn->hasEffect(EEffect::MetaFormAttack) || pawn->hasEffect(EEffect::MetaFormProtection) || pawn->hasEffect(EEffect::MetaFormSupport) || pawn->hasEffect(EEffect::EscapeMode))
        {
            // override speed max
            coefSpeed = pawn->PlayerDataAsset->EscapeModeCoef;
        }
        // set material parameter
        float percent = FMath::Clamp(get()->RU_PercentSpeed * coefSpeed, 0.0f, 2.6f);
        m_postProcessMaterial->SetScalarParameterValue("Weight", percent * 2);
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