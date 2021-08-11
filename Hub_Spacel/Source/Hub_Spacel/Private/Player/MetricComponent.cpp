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
            shipPawn->OnAddEffectDelegate.AddDynamic(this, &UMetricComponent::AddEffect);
            shipPawn->OnFeedbackScoreDelegate.AddDynamic(this, &UMetricComponent::OnScored);
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
        uint8 precision {}, nbKill {};
        uint16 totalScore {};
        if (auto precMetric = static_cast<MetricPrecision<Metric::DataPrecision>*>(m_metric->getData(EMetric::Precision)))
        {
             precision = (uint8)(((float)precMetric->m_nbSuccess / (float)precMetric->m_nb) * 100);
        }

        if (auto killMetric = static_cast<MetricKill<Metric::Data>*>(m_metric->getData(EMetric::Kill)))
        {
            nbKill = killMetric->m_nb;
        }

        if (auto totalScoreMetric = static_cast<MetricScore<Metric::DataScore>*>(m_metric->getData(EMetric::TotalScore)))
        {
            totalScore = totalScoreMetric->m_nb;
        }

        // send data to client
        RPCNetMulticastSendData(precision, nbKill, totalScore);
    }
}

void UMetricComponent::RPCNetMulticastSendData_Implementation(uint8 _precision, uint8 _nbKill, uint16 _totalScore)
{
    this->Precision = _precision;
    this->NbKill = _nbKill;
    this->TotalScore = _totalScore;
    this->HasInit = true;
}

void UMetricComponent::OnScored(EScoreType _type, int32 _value)
{
    switch (_type)
    {
        case EScoreType::Kill:
        {
            createKillData();
            break;
        }

        case EScoreType::Emp:
        {
            createEmpPointData((uint16)_value);
            break;
        }

        case EScoreType::Tank:
        {
            createTankPointData((uint16)_value);
            break;
        }
    }

    createTotalPointData((uint16)_value);
}

void UMetricComponent::AddEffect(EEffect _effect)
{
    if (_effect == EEffect::Fog)
    {
        createFogData();
    }
}

void UMetricComponent::createPrecisionData(bool _success)
{
    Metric::DataPrecision data;
    data.success = _success;
    m_metric->operator()(EMetric::Precision, std::move(data));
}

void UMetricComponent::createFogData()
{
    Metric::Data data{};
    m_metric->operator()(EMetric::Fog, std::move(data));
}

void UMetricComponent::createKillData()
{
    Metric::Data data{};
    m_metric->operator()(EMetric::Kill, std::move(data));
}

void UMetricComponent::createEmpPointData(uint16 _value)
{
    Metric::DataScore data;
    data.value = _value;
    m_metric->operator()(EMetric::EmpPoint, std::move(data));
}

void UMetricComponent::createTankPointData(uint16 _value)
{
    Metric::DataScore data;
    data.value = _value;
    m_metric->operator()(EMetric::TankPoint, std::move(data));
}

void UMetricComponent::createMatiereWinData(uint16 _value)
{
    Metric::DataMatiere data;
    data.value = _value;
    m_metric->operator()(EMetric::MatiereWin, std::move(data));
}

void UMetricComponent::createMatiereRepair(uint16 _value)
{
    Metric::DataMatiere data;
    data.value = _value;
    m_metric->operator()(EMetric::MatiereUseForRepair, std::move(data));
}

void UMetricComponent::createTotalPointData(uint16 _value)
{
    Metric::DataScore data;
    data.value = _value;
    m_metric->operator()(EMetric::TotalScore, std::move(data));
}
