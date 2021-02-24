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
    if (!m_shipPawnOwner.IsValid() && !initShipPawnOwner()) return;

    m_shipPawnOwner.Get()->OnUpdateMatiereDelegate.AddDynamic(this, &URepairComponent::OnUpdateMatiere);
    m_shipPawnOwner.Get()->OnHitProtectionDelegate.AddDynamic(this, &URepairComponent::OnHitProtection);
    m_shipPawnOwner.Get()->OnHitSupportDelegate.AddDynamic(this, &URepairComponent::OnHitSupport);
    m_shipPawnOwner.Get()->OnRepairProtectionDelegate.AddDynamic(this, &URepairComponent::OnRepairProtection);
    m_shipPawnOwner.Get()->OnRepairSupportDelegate.AddDynamic(this, &URepairComponent::OnRepairSupport);
}

void URepairComponent::OnUpdateMatiere(int _value)
{
    m_shipPawnOwner.Get()->RU_Matiere += _value;
    m_shipPawnOwner.Get()->OnRep_Matiere();
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
    repair(m_shipPawnOwner.Get()->ModuleComponent->R_RemovedProtectionLocations, m_shipPawnOwner.Get()->ModuleComponent->RU_ProtectionLocations,
        std::bind(&UModuleComponent::OnRep_Protection, m_shipPawnOwner.Get()->ModuleComponent), this->RepairProtectionHandle);
}

void URepairComponent::OnRepairSupport()
{
    m_isRepairSupport = !m_isRepairSupport;
    onRepair(m_isRepairSupport, this->RepairSupportHandle, &URepairComponent::RepairSupport);
}

void URepairComponent::RepairSupport()
{
    repair(m_shipPawnOwner.Get()->ModuleComponent->R_RemovedSupportLocations, m_shipPawnOwner.Get()->ModuleComponent->RU_SupportLocations,
        std::bind(&UModuleComponent::OnRep_Support, m_shipPawnOwner.Get()->ModuleComponent), this->RepairSupportHandle);
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
        if (m_shipPawnOwner.Get()->RU_Matiere > 0)
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