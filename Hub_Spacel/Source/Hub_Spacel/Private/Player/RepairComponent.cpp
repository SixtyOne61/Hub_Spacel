// Fill out your copyright notice in the Description page of Project Settings.


#include "RepairComponent.h"
#include "Player/ShipPawn.h"
#include "Player/ModuleComponent.h"
#include "Player/LocalPlayerActionComponent.h"
#include "DataAsset/PlayerDataAsset.h"
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

void URepairComponent::OnUpdateMatiere(int _value)
{
    if (get<AShipPawn>() != nullptr)
    {
        get<AShipPawn>()->addMatiere(_value);
    }
}

ESkillReturn URepairComponent::onRepairProtection()
{
    if (get() != nullptr)
    {
        if (get<AShipPawn>() != nullptr && get<AShipPawn>()->PlayerDataAsset != nullptr)
        {
            int min = get<AShipPawn>()->PlayerDataAsset->ProtectionRatioMatiere;
            int effect = get<AShipPawn>()->PlayerDataAsset->ProtectionRatioEffect;
            return repair(get()->ModuleComponent->RemovedProtectionLocations, get()->ModuleComponent->RU_ProtectionLocations, std::bind(&UModuleComponent::OnRep_Protection, get()->ModuleComponent), min, effect);
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
    else if (get()->ModuleComponent->RemovedSupportLocations.Num() != 0)
    {
        lb(get()->ModuleComponent->RemovedSupportLocations,
            get()->ModuleComponent->RU_SupportLocations,
            std::bind(&UModuleComponent::OnRep_Support, get()->ModuleComponent));
    }
    else
    {
        this->OnUpdateMatiere(_value);
    }
}

ESkillReturn URepairComponent::onRepairSupport()
{
    if (get() != nullptr)
    {
        if (get<AShipPawn>() != nullptr && get<AShipPawn>()->PlayerDataAsset != nullptr)
        {
            int min = get<AShipPawn>()->PlayerDataAsset->SupportRatioMatiere;
            int effect = get<AShipPawn>()->PlayerDataAsset->SupportRatioEffect;
            return repair(get()->ModuleComponent->RemovedSupportLocations, get()->ModuleComponent->RU_SupportLocations, std::bind(&UModuleComponent::OnRep_Support, get()->ModuleComponent), min, effect);
        }
    }
    return ESkillReturn::InternError;
}

ESkillReturn URepairComponent::repair(TArray<FVector_NetQuantize>& _removedLocations, TArray<FVector_NetQuantize>& _locations, std::function<void(void)> _onRep, int _minMatiere, int _effect)
{
    if (_removedLocations.Num() != 0)
    {
        if (AShipPawn* pawn = get<AShipPawn>())
        {
            if (pawn->RU_Matiere >= _minMatiere)
            {
                uint8 nbRepair { 0 };

                while (_effect > 0 && _removedLocations.Num() > 0)
                {
                    _locations.Add(_removedLocations[0]);
                    _removedLocations.RemoveAt(0);
                    _effect--;
                    ++nbRepair;
                }

                RPCClientRepair(nbRepair);

                this->OnUpdateMatiere(-1 * _minMatiere);
                _onRep();
                return ESkillReturn::Success;
            }
        }

        return ESkillReturn::NoMater;
    }
    return ESkillReturn::Unavailable;
}

void URepairComponent::RPCClientRepair_Implementation(uint8 _value)
{
    if (AShipPawn* pawn = get<AShipPawn>())
    {
        if (ULocalPlayerActionComponent* component = Cast<ULocalPlayerActionComponent>(pawn->GetComponentByClass(ULocalPlayerActionComponent::StaticClass())))
        {
            component->createMatiereRepair(_value);
        }
    }
}