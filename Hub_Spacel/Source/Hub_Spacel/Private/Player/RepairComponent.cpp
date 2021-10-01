// Fill out your copyright notice in the Description page of Project Settings.


#include "RepairComponent.h"
#include "Player/ShipPawn.h"
#include "Player/ModuleComponent.h"
#include "Player/MetricComponent.h"
#include "DataAsset/PlayerDataAsset.h"
#include "DataAsset/UniqueSkillDataAsset.h"
#include "Net/UnrealNetwork.h"

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

            int32 lostProtection = moduleComponent->RemovedProtectionLocations.Num();
            int32 lostEngine = moduleComponent->RemovedSupportLocations.Num();

            if(lostProtection == 0 && lostEngine == 0) return ESkillReturn::Unavailable;

            float ratio = pawn->PlayerDataAsset->RepairRatio;
            uint32 matiereUse = 0;

            uint32 repair = this->RepairSkillDataAsset->Value * ratio;
            int deltaMatiere = 1.0f / ratio;

            auto lb = [&repair, &matiereUse, &deltaMatiere](int32& _lost, TArray<FVector_NetQuantize>& _removedLocations, TArray<FVector_NetQuantize>& _locations) -> bool
            {
                if (repair != 0 && _lost != 0)
                {
                    _locations.Add(_removedLocations[0]);
                    _removedLocations.RemoveAt(0);
                    --_lost;
                    --repair;
                    matiereUse += deltaMatiere;
                    return true;
                }
                return false;
            };

            bool hasRepairProtection = false;
            bool hasRepairSupport = false;

            while (repair != 0 && !(lostProtection == 0 && lostEngine == 0))
            {
                hasRepairProtection |= lb(lostProtection, moduleComponent->RemovedProtectionLocations, moduleComponent->RU_ProtectionLocations);
                hasRepairSupport |= lb(lostEngine, moduleComponent->RemovedSupportLocations, moduleComponent->RU_SupportLocations);
            }

            if (UMetricComponent* component = Cast<UMetricComponent>(pawn->GetComponentByClass(UMetricComponent::StaticClass())))
            {
                component->updateMetric<SMetricAdd, uint8>(EMetric::MatiereUseForRepair, { matiereUse });
            }

            this->OnUpdateMatiere(-1 * matiereUse, EMatiereOrigin::Lost);

            if (hasRepairProtection) moduleComponent->OnRep_Protection();
            if (hasRepairSupport) moduleComponent->OnRep_Support();

            return ESkillReturn::Success;
        }
    }

    return ESkillReturn::InternError;
}

void URepairComponent::heal(uint8 _value)
{
    auto lb = [&](TArray<FVector_NetQuantize>& _removedLocations, TArray<FVector_NetQuantize>& _locations, std::function<void(void)> _onRep)
    {
        while (_value > 0 && _removedLocations.Num() > 0)
        {
            _locations.Add(_removedLocations[0]);
            _removedLocations.RemoveAt(0);
            _value--;
        }

        _onRep();
    };

    if (get()->ModuleComponent->RemovedProtectionLocations.Num() != 0)
    {
        lb(get()->ModuleComponent->RemovedProtectionLocations,
            get()->ModuleComponent->RU_ProtectionLocations,
            std::bind(&UModuleComponent::OnRep_Protection, get()->ModuleComponent));
    }

    if (get()->ModuleComponent->RemovedSupportLocations.Num() != 0)
    {
        lb(get()->ModuleComponent->RemovedSupportLocations,
            get()->ModuleComponent->RU_SupportLocations,
            std::bind(&UModuleComponent::OnRep_Support, get()->ModuleComponent));
    }

    if (_value != 0)
    {
        this->OnUpdateMatiere(_value, EMatiereOrigin::Heal);
    }
}
