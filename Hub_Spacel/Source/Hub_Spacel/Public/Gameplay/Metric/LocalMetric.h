// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enum/SpacelEnum.h"
#include "Util/Any.h"
#include <memory>
#include <map>
#include <algorithm>
#include <tuple>

struct HUB_SPACEL_API SMetricIncrease
{
	inline constexpr void operator()()
	{
		m_nb++;
	}

	inline constexpr std::tuple<int> get() const
	{
		return { m_nb };
	}

private:
	int m_nb { 0 };
};

struct HUB_SPACEL_API SMetricRatio
{
	inline constexpr void operator()(std::tuple<bool>&& _vals)
	{
		++m_nb;
		if (std::get<0>(_vals))
		{
			++m_nbSuccess;
		}
	}

	inline constexpr std::tuple<int, float> get() const
	{
		return { m_nb, ((float)m_nbSuccess / (float)m_nb) * 100 };
	}

private:
	int m_nb { 0 };
	int m_nbSuccess { 0 };
};

struct HUB_SPACEL_API SMetricAdd
{
	inline constexpr void operator()(std::tuple<int> && _vals)
	{
		m_nb += std::get<0>(_vals);
	}

	inline constexpr std::tuple<int> get() const
	{
		return { m_nb };
	}

private:
	int m_nb { 0 };
};

/**
 * 
 */
class HUB_SPACEL_API LocalMetric
{
public:
	LocalMetric()
	{
		m_metric.insert({ EMetric::Fog, SMetricIncrease {} });
		m_metric.insert({ EMetric::Precision, SMetricRatio {} });
		m_metric.insert({ EMetric::Kill, SMetricIncrease {} });
		m_metric.insert({ EMetric::EmpPoint, SMetricAdd {} });
		m_metric.insert({ EMetric::TankPoint, SMetricAdd {} });
		m_metric.insert({ EMetric::MatiereWin, SMetricAdd {} });
		m_metric.insert({ EMetric::MatiereUseForRepair, SMetricAdd {} });
		m_metric.insert({ EMetric::TotalScore, SMetricAdd {} });
		m_metric.insert({ EMetric::ShieldedEnemyDamage, SMetricAdd {} });
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
