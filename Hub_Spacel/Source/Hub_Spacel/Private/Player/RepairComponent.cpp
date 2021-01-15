// Fill out your copyright notice in the Description page of Project Settings.


#include "RepairComponent.h"
#include "Player/ShipPawn.h"
#include "Player/PlayerShipController.h"
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
    RU_Matiere += _value;
    OnRep_Matiere();
}

void URepairComponent::OnHitProtection()
{

}

void URepairComponent::OnHitSupport()
{

}

void URepairComponent::OnRep_Matiere()
{
    // Update UI
}

void URepairComponent::onRepair(bool _on, FTimerHandle & _handle, void(URepairComponent::* _callback)())
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    if (_on)
    {
        world->GetTimerManager().SetTimer(_handle, this, _callback, 0.2f, true, 0.0f);
    }
    else
    {
        world->GetTimerManager().ClearTimer(_handle);
    }
}

void URepairComponent::OnRepairProtection(bool _on)
{
    onRepair(_on, this->RepairProtectionHandle, &URepairComponent::RepairProtection);
}

void URepairComponent::RepairProtection()
{
    repair(m_shipPawnOwner.Get()->ModuleComponent->R_RemovedProtectionLocations, m_shipPawnOwner.Get()->ModuleComponent->RU_ProtectionLocations,
        std::bind(&UModuleComponent::OnRep_Protection, m_shipPawnOwner.Get()->ModuleComponent));
}

void URepairComponent::OnRepairSupport(bool _on)
{
    onRepair(_on, this->RepairSupportHandle, &URepairComponent::RepairSupport);
}

void URepairComponent::RepairSupport()
{
    repair(m_shipPawnOwner.Get()->ModuleComponent->R_RemovedSupportLocations, m_shipPawnOwner.Get()->ModuleComponent->RU_SupportLocations,
        std::bind(&UModuleComponent::OnRep_Support, m_shipPawnOwner.Get()->ModuleComponent));
}

void URepairComponent::repair(TArray<FVector>& _removedLocations, TArray<FVector>& _locations, std::function<void(void)> _onRep)
{
    if (_removedLocations.Num() != 0)
    {
        if (this->RU_Matiere > 0)
        {
            _locations.Add(_removedLocations[0]);
            _removedLocations.RemoveAt(0);
            this->OnUpdateMatiere(-1);
            _onRep();
        }
        else
        {
            // feedback matiere empty
        }
    }
    else
    {
        // feedback full repair
    }
}

void URepairComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(URepairComponent, RU_Matiere);
}