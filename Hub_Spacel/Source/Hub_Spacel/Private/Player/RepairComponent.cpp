// Fill out your copyright notice in the Description page of Project Settings.


#include "RepairComponent.h"
#include "Player/ShipPawn.h"
#include "Player/ModuleComponent.h"
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
        pawn->OnHitProtectionDelegate.AddDynamic(this, &URepairComponent::OnHitProtection);
        pawn->OnHitSupportDelegate.AddDynamic(this, &URepairComponent::OnHitSupport);
        pawn->OnRepairProtectionDelegate.AddDynamic(this, &URepairComponent::OnRepairProtection);
        pawn->OnRepairSupportDelegate.AddDynamic(this, &URepairComponent::OnRepairSupport);
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

void URepairComponent::OnHitProtection()
{

}

void URepairComponent::OnHitSupport()
{

}

void URepairComponent::onRepair(bool _on, FTimerHandle & _handle, void(URepairComponent::* _callback)())
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    if (_on)
    {
        world->GetTimerManager().SetTimer(_handle, this, _callback, 0.25f, true, 0.0f);
    }
    else
    {
        world->GetTimerManager().ClearTimer(_handle);
    }
}

void URepairComponent::OnRepairProtection()
{
    m_isRepairProtection = !m_isRepairProtection;
    onRepair(m_isRepairProtection, this->RepairProtectionHandle, &URepairComponent::RepairProtection);
}

void URepairComponent::RepairProtection()
{
    repair(get()->ModuleComponent->R_RemovedProtectionLocations, get()->ModuleComponent->RU_ProtectionLocations,
        std::bind(&UModuleComponent::OnRep_Protection, get()->ModuleComponent), this->RepairProtectionHandle);
}

void URepairComponent::OnRepairSupport()
{
    m_isRepairSupport = !m_isRepairSupport;
    onRepair(m_isRepairSupport, this->RepairSupportHandle, &URepairComponent::RepairSupport);
}

void URepairComponent::RepairSupport()
{
    repair(get()->ModuleComponent->R_RemovedSupportLocations, get()->ModuleComponent->RU_SupportLocations,
        std::bind(&UModuleComponent::OnRep_Support, get()->ModuleComponent), this->RepairSupportHandle);
}

void URepairComponent::repair(TArray<FVector>& _removedLocations, TArray<FVector>& _locations, std::function<void(void)> _onRep, FTimerHandle & _handle)
{
    auto lb_clearTimer = [&]()
    {
        UWorld* world{ this->GetWorld() };
        if (!ensure(world != nullptr)) return;
        // feedback matiere empty
        world->GetTimerManager().ClearTimer(_handle);
    };

    if (_removedLocations.Num() != 0)
    {
        if (get<AShipPawn>() != nullptr && get<AShipPawn>()->RU_Matiere > 0)
        {
            _locations.Add(_removedLocations[0]);
            _removedLocations.RemoveAt(0);
            this->OnUpdateMatiere(-1);
            _onRep();
        }
        else
        {
            lb_clearTimer();
        }
    }
    else
    {
        lb_clearTimer();
    }
}

void URepairComponent::kill()
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    auto lb_clean = [&world](FTimerHandle& _handle, bool& _toggle)
    {
        if (_toggle)
        {
            _toggle = false;
            world->GetTimerManager().ClearTimer(_handle);
        }
    };

    lb_clean(this->RepairProtectionHandle, this->m_isRepairProtection);
    lb_clean(this->RepairSupportHandle, this->m_isRepairSupport);
}