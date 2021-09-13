// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/PlayerActorComponent.h"
#include "Gameplay/Metric/LocalMetric.h"
#include "Enum/SpacelEnum.h"
#include "MetricComponent.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UMetricComponent : public UPlayerActorComponent
{
	GENERATED_BODY()

public:
	void BeginPlay() override;

	template<class DataType, class ... ts>
	inline void updateMetric(EMetric _type, std::tuple<ts...>&& _vals)
	{
		using Ttuple = std::tuple<ts...>;

		if (auto* data = m_metric->getData<DataType>(_type))
		{
			data->operator()(std::forward<Ttuple>(_vals));
		}
	}

	template<class DataType = SMetricIncrease>
	inline void updateMetric(EMetric _type)
	{
		if (auto* data = m_metric->getData<DataType>(_type))
		{
			data->operator()();
		}
	}

	UFUNCTION()
	void OnScored(EScoreType _type, int32 _value);

private:
	UFUNCTION()
	void AddEffect(EEffect _effect);

	UFUNCTION()
	void OnChangeState(EGameState _state);

	UFUNCTION(Reliable, NetMulticast)
	void RPCNetMulticastSendData(uint8 _precision, uint8 _nbKill, uint16 _totalScore);
	
	UFUNCTION(Reliable, Client)
	void RPCClientSendData(uint8 _nbFog, uint16 _empPoint, uint16 _tankPoint, uint16 _matiereWin, uint16 _matiereUseForRepair);

public:
	UPROPERTY()
	uint8 Precision { 0 };

	UPROPERTY()
	uint8 NbKill { 0 };

	UPROPERTY()
	uint16 TotalScore { 0 };

	UPROPERTY()
	uint8 NbFog { 0 };

	UPROPERTY()
	uint16 EmpPoint { 0 };

	UPROPERTY()
	uint16 TankPoint { 0 };

	UPROPERTY()
	uint16 MatiereWin { 0 };

	UPROPERTY()
	uint16 MatiereUseForRepair { 0 };

	UPROPERTY()
	bool HasInit { false };

private:
	std::unique_ptr<LocalMetric> m_metric{ nullptr };
};
