// Fill out your copyright notice in the Description page of Project Settings.


#include "MetricComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShipPawn.h"
#include "GameState/SpacelGameState.h"

void UMetricComponent::BeginPlay()
{
    Super::BeginPlay();

    if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        m_metric = std::make_unique<LocalMetric>();

        if (AShipPawn* shipPawn = get<AShipPawn>())
        {
            // TO DO : doesn't work add effect is client side event
            shipPawn->OnAddEffectDelegate.AddDynamic(this, &UMetricComponent::AddEffect);
        }

        if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())))
        {
            spacelGameState->OnChangeStateDelegate.AddDynamic(this, &UMetricComponent::OnChangeState);
        }
    }
}

void UMetricComponent::OnChangeState(EGameState _state)
{
    if (_state == EGameState::EndGame)
    {
        // TO DO : refacto this all big bad part
        uint8 precision {}, nbKill {};
        uint16 totalScore {};

        if (auto* data = m_metric->getData<SMetricRatio>(EMetric::Precision))
        {
            auto const& result = data->get();
            precision = (uint8)(std::get<1>(result));
        }
        
        if (auto* data = m_metric->getData<SMetricIncrease>(EMetric::Kill))
        {
            auto const& result = data->get();
            nbKill = (uint8)(std::get<0>(result));
        }
        
        if (auto* data = m_metric->getData<SMetricAdd>(EMetric::TotalScore))
        {
            auto const& result = data->get();
            totalScore = (uint8)(std::get<0>(result));
        }

        // send data to all client
        RPCNetMulticastSendData(precision, nbKill, totalScore);

        uint8 nbFog {};
        uint16 empPoint {}, tankPoint {}, matiereWin {}, matiereUseForRepair {};
        if (auto* data = m_metric->getData<SMetricIncrease>(EMetric::Fog))
        {
            auto const& result = data->get();
            nbFog = std::get<0>(result);
        }
        
        if (auto* data = m_metric->getData<SMetricAdd>(EMetric::EmpPoint))
        {
            auto const& result = data->get();
            empPoint = std::get<0>(result);
        }
        
        if (auto* data = m_metric->getData<SMetricAdd>(EMetric::TankPoint))
        {
            auto const& result = data->get();
            tankPoint = std::get<0>(result);
        }
        
        if (auto* data = m_metric->getData<SMetricAdd>(EMetric::MatiereWin))
        {
            auto const& result = data->get();
            matiereWin = std::get<0>(result);
        }
        
        if (auto* data = m_metric->getData<SMetricAdd>(EMetric::MatiereUseForRepair))
        {
            auto const& result = data->get();
            matiereUseForRepair = std::get<0>(result);
        }

        RPCClientSendData(nbFog, empPoint, tankPoint, matiereWin, matiereUseForRepair);
    }
}

void UMetricComponent::RPCNetMulticastSendData_Implementation(uint8 _precision, uint8 _nbKill, uint16 _totalScore)
{
    this->Precision = _precision;
    this->NbKill = _nbKill;
    this->TotalScore = _totalScore;
    this->HasInit = true;
}

void UMetricComponent::RPCClientSendData_Implementation(uint8 _nbFog, uint16 _empPoint, uint16 _tankPoint, uint16 _matiereWin, uint16 _matiereUseForRepair)
{
    this->NbFog = _nbFog;
    this->EmpPoint = _empPoint;
    this->TankPoint = _tankPoint;
    this->MatiereWin = _matiereWin;
    this->MatiereUseForRepair = _matiereUseForRepair;
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

        case EScoreType::Tank:
        {
            updateMetric<SMetricAdd, uint16>(EMetric::TankPoint, { (uint16)_value });
            break;
        }
    }

    updateMetric<SMetricAdd, uint16>(EMetric::TotalScore, { (uint16)_value });
}

void UMetricComponent::AddEffect(EEffect _effect)
{
    if (_effect == EEffect::Fog)
    {
        updateMetric<SMetricIncrease>(EMetric::Fog);
    }
}

