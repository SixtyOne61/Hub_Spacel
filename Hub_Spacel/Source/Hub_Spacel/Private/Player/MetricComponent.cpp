// Fill out your copyright notice in the Description page of Project Settings.


#include "MetricComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShipPawn.h"
#include "GameState/SpacelGameState.h"

void UMetricComponent::BeginPlay()
{
    Super::BeginPlay();

    ENetMode mode = this->GetNetMode();
    m_metric = std::make_unique<LocalMetric>(mode,
        std::bind(&UMetricComponent::RPCNetMulticastSendVoidData, this, std::placeholders::_1),
        std::bind(&UMetricComponent::RPCNetMulticastSendBoolData, this, std::placeholders::_1, std::placeholders::_2),
        std::bind(&UMetricComponent::RPCNetMulticastSendIntData, this, std::placeholders::_1, std::placeholders::_2));

    if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        if (AShipPawn* shipPawn = get<AShipPawn>())
        {
            shipPawn->OnAddEffectServerDelegate.AddDynamic(this, &UMetricComponent::AddEffect);
        }
    }
}

void UMetricComponent::RPCNetMulticastSendVoidData_Implementation(EMetric _type)
{
    if(this->GetNetMode() == ENetMode::NM_DedicatedServer) return;

    updateMetric<SMetricIncrease>(_type);
}

void UMetricComponent::RPCNetMulticastSendBoolData_Implementation(EMetric _type, bool _data)
{
    if (this->GetNetMode() == ENetMode::NM_DedicatedServer) return;

    updateMetric<SMetricRatio, bool>(_type, {_data});
}

void UMetricComponent::RPCNetMulticastSendIntData_Implementation(EMetric _type, int _data)
{
    if (this->GetNetMode() == ENetMode::NM_DedicatedServer) return;

    updateMetric<SMetricAdd, uint16>(_type, {(uint16)_data});
}

void UMetricComponent::OnScored(EScoreType _type, int32 _value)
{
    switch (_type)
    {
        case EScoreType::Kill:
        {
            updateMetric<SMetricIncrease>(EMetric::Kill);
            break;
        }

        case EScoreType::Emp:
        {
            updateMetric<SMetricAdd, uint16>(EMetric::EmpPoint, { (uint16)_value });
            break;
        }
    }

    updateMetric<SMetricAdd, uint16>(EMetric::TotalScore, { (uint16)_value });
}

void UMetricComponent::AddEffect(EEffect _effect)
{
    if (_effect == EEffect::Killed)
    {
        updateMetric<SMetricIncrease>(EMetric::Death);
    }
}

