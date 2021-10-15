// Fill out your copyright notice in the Description page of Project Settings.


#include "RepairComponent.h"
#include "Player/ShipPawn.h"
#include "Player/ModuleComponent.h"
#include "Player/MetricComponent.h"
#include "DataAsset/PlayerDataAsset.h"
#include "DataAsset/UniqueSkillDataAsset.h"
#include "Net/UnrealNetwork.h"
#include "Mesh/SpacelInstancedMeshComponent.h"

URepairComponent::URepairComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URepairComponent::BeginPlay()
{
    Super::BeginPlay();

    if (get() == nullptr && !initShipPawnOwner()) return;
    if (this->GetNetMode() != ENetMode::NM_DedicatedServer) return;

    if (AShipPawn* pawn = get<AShipPawn>())
    {
        pawn->OnUpdateMatiereDelegate.AddDynamic(this, &URepairComponent::OnUpdateMatiere);
    }
}

void URepairComponent::OnUpdateMatiere(int _value, EMatiereOrigin _type)
{
    if (get<AShipPawn>() != nullptr)
    {
        get<AShipPawn>()->addMatiere(_value, _type);
    }
}

ESkillReturn URepairComponent::onRepair()
{
    if(this->RepairSkillDataAsset == nullptr) return ESkillReturn::InternError;

    if (AShipPawn* pawn = get<AShipPawn>())
    {
        if(pawn->PlayerDataAsset == nullptr) return ESkillReturn::InternError;

        if (UModuleComponent* moduleComponent = pawn->ModuleComponent)
        {
            if(pawn->RU_Matiere < (int16)this->RepairSkillDataAsset->MatiereNeeded) return ESkillReturn::NoMater;

            if (moduleComponent->ProtectionComponent == nullptr) return ESkillReturn::InternError;
            if (moduleComponent->SupportComponent == nullptr) return ESkillReturn::InternError;

            if (moduleComponent->ProtectionComponent->GetNbRemoved() == 0 && moduleComponent->SupportComponent->GetNbRemoved() == 0)
            {
                return ESkillReturn::Unavailable;
            }

            this->OnUpdateMatiere(-1 * this->RepairSkillDataAsset->MatiereNeeded, EMatiereOrigin::Lost);
            if (UMetricComponent* component = Cast<UMetricComponent>(pawn->GetComponentByClass(UMetricComponent::StaticClass())))
            {
                component->updateMetric<SMetricAdd, uint8>(EMetric::MatiereUseForRepair, { this->RepairSkillDataAsset->MatiereNeeded });
            }

            float ratio = pawn->PlayerDataAsset->RepairRatio;
            int maxRepair = this->RepairSkillDataAsset->Value * ratio;

            repair(maxRepair, moduleComponent);
            return ESkillReturn::Success;
        }
    }

    return ESkillReturn::InternError;
}

int URepairComponent::repair(int _nb, UModuleComponent *& _moduleComponent) const
{
    int maxRepairProtection = _nb * (2.0f / 3.0f); // TO DO expose this to player setting ?
    int maxRepairSupport = _nb - maxRepairProtection;

    // keep rest of protection repair for increase support rest
    maxRepairSupport += _moduleComponent->ProtectionComponent->Repair(maxRepairProtection);
    // call repair support, and use rest for increase repair protection
    return _moduleComponent->ProtectionComponent->Repair(_moduleComponent->SupportComponent->Repair(maxRepairSupport));
}

void URepairComponent::heal(uint8 _value)
{
    int rest = repair(_value, get()->ModuleComponent);
    if (rest != 0)
    {
        this->OnUpdateMatiere(rest, EMatiereOrigin::Heal);
    }
}
