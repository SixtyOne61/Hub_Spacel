// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enum/SpacelEnum.h"
#include <memory>
#include <map>
#include <algorithm>

namespace Metric
{
	struct Data
	{
	};

	struct DataPrecision : Data
	{
		bool success { false };
	};

	struct DataScore : Data
	{
		uint16 value { };
	};

	struct DataMatiere : Data
	{
		uint16 value { };
	};
}

class HUB_SPACEL_API MetricInterface
{
public:
	MetricInterface() {};
	virtual ~MetricInterface() {};
	MetricInterface(MetricInterface const&) = delete;

	virtual void operator()(Metric::Data && _data) = 0;
};

template<class T>
class HUB_SPACEL_API MetricFog : public MetricInterface
{
	void operator()(Metric::Data && _data) override
	{
		m_nb++;
	}

public:
	int m_nb {};
};

template<class T>
class HUB_SPACEL_API MetricPrecision : public MetricInterface
{
	void operator()(Metric::Data&& _data) override
	{
		T const& data = static_cast<T const&>(_data);
		++m_nb;
		if (data.success)
		{
			++m_nbSuccess;
		}
	}

public:
	int m_nb { 0 };
	int m_nbSuccess { 0 };
};

template<class T>
class HUB_SPACEL_API MetricKill : public MetricInterface
{
	void operator()(Metric::Data&& _data) override
	{
		++m_nb;
	}

public:
	int m_nb{ 0 };
};

template<class T>
class HUB_SPACEL_API MetricScore : public MetricInterface
{
	void operator()(Metric::Data&& _data) override
	{
		T const& data = static_cast<T const&>(_data);
		m_nb += data.value;
	}

public:
	int m_nb { 0 };
};

template<class T>
class HUB_SPACEL_API MetricMatiere : public MetricInterface
{
	void operator()(Metric::Data&& _data) override
	{
		T const& data = static_cast<T const&>(_data);
		m_nb += data.value;
	}

public:
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
		m_metric.insert({ EMetric::Fog, std::make_shared<MetricFog<Metric::Data>>() });
		m_metric.insert({ EMetric::Precision, std::make_shared<MetricPrecision<Metric::DataPrecision>>() });
		m_metric.insert({ EMetric::Kill, std::make_shared<MetricKill<Metric::Data>>() });
		m_metric.insert({ EMetric::EmpPoint, std::make_shared<MetricScore<Metric::DataScore>>() });
		m_metric.insert({ EMetric::TankPoint, std::make_shared<MetricScore<Metric::DataScore>>() });
		m_metric.insert({ EMetric::MatiereWin, std::make_shared<MetricMatiere<Metric::DataMatiere>>() });
		m_metric.insert({ EMetric::MatiereUseForRepair, std::make_shared<MetricMatiere<Metric::DataMatiere>>() });
		m_metric.insert({ EMetric::TotalScore, std::make_shared<MetricScore<Metric::DataScore>>() });
	}

	~LocalMetric()
	{

	}

	void operator()(EMetric _type, Metric::Data && _data)
	{
		std::for_each(m_metric.begin(), m_metric.end(), [&_type, &_data](auto _obj)
		{
			if(_obj.first == _type)
			{
				_obj.second->operator()(std::forward<Metric::Data>(_data));
			}
		});
	}

	MetricInterface* getData(EMetric _type)
	{
		for (auto& metric : m_metric)
		{
			if (metric.first == _type)
			{
				return metric.second.get();
			}
		}

		return nullptr;
	}

protected:
	std::map<EMetric, std::shared_ptr<MetricInterface>> m_metric;
};
