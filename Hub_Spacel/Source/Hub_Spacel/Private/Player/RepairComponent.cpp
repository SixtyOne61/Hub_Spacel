// Fill out your copyright notice in the Description page of Project Settings.


#include "RepairComponent.h"
#include "Player/ShipPawn.h"
#include "Player/ModuleComponent.h"
#include "DataAsset/PlayerDataAsset.h"
#include "Net/UnrealNetwork.h"

URepairComponent::URepairComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void URepairComponent::BeginPlay()
{
    Super::BeginPlay();

    if (this->GetNetMode() != ENetMode::NM_DedicatedServer) return;
    if (get() == nullptr && !initShipPawnOwner()) return;

    if (AShipPawn* pawn = get<AShipPawn>())
    {
        pawn->OnUpdateMatiereDelegate.AddDynamic(this, &URepairComponent::OnUpdateMatiere);
    }
}

void URepairComponent::OnUpdateMatiere(int _value)
{
    if (get<AShipPawn>() != nullptr)
    {
        get<AShipPawn>()->RU_Matiere += _value;
        get<AShipPawn>()->OnRep_Matiere();
    }
}

bool URepairComponent::onRepairProtection()
{
    if (get() != nullptr)
    {
        if (get<AShipPawn>() != nullptr && get<AShipPawn>()->PlayerDataAsset != nullptr)
        {
            int min = get<AShipPawn>()->PlayerDataAsset->ProtectionRatioMatiere;
            int effect = get<AShipPawn>()->PlayerDataAsset->ProtectionRatioEffect;
            return repair(get()->ModuleComponent->R_RemovedProtectionLocations, get()->ModuleComponent->RU_ProtectionLocations, std::bind(&UModuleComponent::OnRep_Protection, get()->ModuleComponent), min, effect);
        }
    }
    return false;
}

bool URepairComponent::onRepairSupport()
{
    if (get() != nullptr)
    {
        if (get<AShipPawn>() != nullptr && get<AShipPawn>()->PlayerDataAsset != nullptr)
        {
            int min = get<AShipPawn>()->PlayerDataAsset->SupportRatioMatiere;
            int effect = get<AShipPawn>()->PlayerDataAsset->SupportRatioEffect;
            return repair(get()->ModuleComponent->R_RemovedSupportLocations, get()->ModuleComponent->RU_SupportLocations, std::bind(&UModuleComponent::OnRep_Support, get()->ModuleComponent), min, effect);
        }
    }
    return false;
}

bool URepairComponent::repair(TArray<FVector>& _removedLocations, TArray<FVector>& _locations, std::function<void(void)> _onRep, int _minMatiere, int _effect)
{
    if (_removedLocations.Num() != 0)
    {
        if (get<AShipPawn>() != nullptr && get<AShipPawn>()->RU_Matiere >= _minMatiere)
        {
            while (_effect > 0 && _removedLocations.Num() > 0)
            {
                _locations.Add(_removedLocations[0]);
                _removedLocations.RemoveAt(0);
                _effect--;
            }

            this->OnUpdateMatiere(-1 * _minMatiere);
            _onRep();
            return true;
        }
    }
    return false;
}

