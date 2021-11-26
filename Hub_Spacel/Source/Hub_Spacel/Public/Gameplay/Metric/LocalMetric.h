// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enum/SpacelEnum.h"
#include "Util/Any.h"
#include <memory>
#include <map>
#include <algorithm>
#include <tuple>
#include <functional>

struct HUB_SPACEL_API SMetricBase
{
	SMetricBase() = default;

	SMetricBase(EMetric _type)
		: m_type(_type)
	{}

protected:
	EMetric m_type {};
};

struct HUB_SPACEL_API SMetricIncrease : SMetricBase
{
	SMetricIncrease() = default;

	SMetricIncrease(EMetric _type, std::function<void(EMetric)> _rpc)
		: SMetricBase(_type)
		, m_rpc(_rpc)
	{}

	~SMetricIncrease() = default;

	inline void operator()()
	{
		m_nb++;
		if (m_rpc != nullptr)
		{
			m_rpc(m_type);
		}
	}

	inline constexpr std::tuple<int> get() const
	{
		return { m_nb };
	}

private:
	int m_nb { 0 };
	std::function<void(EMetric)> m_rpc { nullptr };
};

struct HUB_SPACEL_API SMetricRatio : SMetricBase
{
	SMetricRatio() = default;

	SMetricRatio(EMetric _type, std::function<void(EMetric, bool)> _rpc)
		: SMetricBase(_type)
		, m_rpc(_rpc)
	{}

	~SMetricRatio() = default;

	inline void operator()(std::tuple<bool>&& _vals)
	{
		++m_nb;
		if (std::get<0>(_vals))
		{
			++m_nbSuccess;
		}

		if (m_rpc != nullptr)
		{
			m_rpc(m_type, std::get<0>(_vals));
		}
	}

	inline constexpr std::tuple<int, float> get() const
	{
		return { m_nb, ((float)m_nbSuccess / (float)m_nb) * 100 };
	}

private:
	int m_nb { 0 };
	int m_nbSuccess { 0 };
	std::function<void(EMetric, bool)> m_rpc{ nullptr };
};

struct HUB_SPACEL_API SMetricAdd : SMetricBase
{
	SMetricAdd() = default;

	SMetricAdd(EMetric _type, std::function<void(EMetric, int)> _rpc)
		: SMetricBase(_type)
		, m_rpc(_rpc)
	{}

	~SMetricAdd() = default;

	inline void operator()(std::tuple<int> && _vals)
	{
		m_nb += std::get<0>(_vals);

		if (m_rpc != nullptr)
		{
			m_rpc(m_type, std::get<0>(_vals));
		}
	}

	inline constexpr std::tuple<int> get() const
	{
		return { m_nb };
	}

private:
	int m_nb { 0 };
	std::function<void(EMetric, int)> m_rpc{ nullptr };
};

/**
 * 
 */
class HUB_SPACEL_API LocalMetric
{
public:
	LocalMetric(ENetMode _mode, std::function<void(EMetric)> _rpcVoid, std::function<void(EMetric, bool)> _rpcBool, std::function<void(EMetric, int)> _rpcInt)
	{
		m_metric.insert({ EMetric::Precision, SMetricRatio { EMetric::Precision, _mode == ENetMode::NM_DedicatedServer ? _rpcBool : nullptr } });
		m_metric.insert({ EMetric::Kill, SMetricIncrease { EMetric::Kill, _mode == ENetMode::NM_DedicatedServer ? _rpcVoid : nullptr } });
		m_metric.insert({ EMetric::EmpPoint, SMetricAdd { EMetric::EmpPoint, _mode == ENetMode::NM_DedicatedServer ? _rpcInt : nullptr } });
		m_metric.insert({ EMetric::TankPoint, SMetricAdd { EMetric::TankPoint, _mode == ENetMode::NM_DedicatedServer ? _rpcInt : nullptr } });
		m_metric.insert({ EMetric::MatiereWin, SMetricAdd { EMetric::MatiereWin, _mode == ENetMode::NM_DedicatedServer ? _rpcInt : nullptr } });
		m_metric.insert({ EMetric::MatiereUseForRepair, SMetricAdd { EMetric::MatiereUseForRepair, _mode == ENetMode::NM_DedicatedServer ? _rpcInt : nullptr } });
		m_metric.insert({ EMetric::TotalScore, SMetricAdd { EMetric::TotalScore, _mode == ENetMode::NM_DedicatedServer ? _rpcInt : nullptr } });
		m_metric.insert({ EMetric::Death, SMetricIncrease { EMetric::Death, _mode == ENetMode::NM_DedicatedServer ? _rpcVoid : nullptr } });
		m_metric.insert({ EMetric::Assist, SMetricIncrease { EMetric::Assist, _mode == ENetMode::NM_DedicatedServer ? _rpcVoid : nullptr } });
		m_metric.insert({ EMetric::Heal, SMetricAdd { EMetric::Heal, _mode == ENetMode::NM_DedicatedServer ? _rpcInt : nullptr } });
		m_metric.insert({ EMetric::Destruction, SMetricIncrease { EMetric::Destruction, _mode == ENetMode::NM_DedicatedServer ? _rpcVoid : nullptr } });
	}

	~LocalMetric() = default;

	template<class T>
	inline T* getData(EMetric _type)
	{
		for (auto& metric : m_metric)
		{
			if (metric.first == _type)
			{
				return &metric.second.unsafe_downcast<T>();
			}
		}
		return nullptr;
	}

protected:
	std::map<EMetric, Any> m_metric;
};
