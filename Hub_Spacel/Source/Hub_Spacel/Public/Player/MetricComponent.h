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

	template<class T>
	inline T* getMetric(EMetric _type)
	{
		return m_metric->getData<T>(_type);
	}

	UFUNCTION()
	void OnScored(EScoreType _type, int32 _value);

private:
	UFUNCTION()
	void AddEffect(EEffect _effect);

	UFUNCTION(Reliable, NetMulticast)
	void RPCNetMulticastSendVoidData(EMetric _type);

	UFUNCTION(Reliable, NetMulticast)
	void RPCNetMulticastSendBoolData(EMetric _type, bool _data);

	UFUNCTION(Reliable, NetMulticast)
	void RPCNetMulticastSendIntData(EMetric _type, int _data);

private:
	std::unique_ptr<LocalMetric> m_metric { nullptr };
};
