// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enum/SpacelEnum.h"
#include <memory>
#include <map>

class HUB_SPACEL_API MetricInterface
{
public:
	MetricInterface() {};
	virtual ~MetricInterface() {};
	MetricInterface(MetricInterface const&) = delete;

	template<class ... Ts>
	void operator()(Ts... _args) { ensure(false); } // TO DO : not work because we manipulate metric interface
};

template<class T>
class HUB_SPACEL_API BehaviourMetric : public MetricInterface
{
public:
	template<class ... Ts>
	void operator()(Ts... _args) { dynamic_cast<T*>(this)->apply(_args); }
};

class HUB_SPACEL_API FogMetric : public BehaviourMetric<FogMetric>
{
	void apply()
	{
		int i = 0;
	};
};

/**
 * 
 */
class HUB_SPACEL_API LocalMetric
{
public:
	LocalMetric()
	{
		m_metric.insert({EMetric::Fog, std::make_shared<FogMetric>()});
	}

	~LocalMetric()
	{

	}

	template<class ... Ts>
	void operator()(EMetric _type, Ts... _args)
	{
		if (m_metric.find(_type) != m_metric.end())
		{
			m_metric[_type]->operator()(_args...);
		}
		else
		{
			ensure(false);
		}
	}

protected:
	std::map<EMetric, std::shared_ptr<MetricInterface>> m_metric;
};
