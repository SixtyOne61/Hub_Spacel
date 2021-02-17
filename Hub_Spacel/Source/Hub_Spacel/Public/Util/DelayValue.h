// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class HUB_SPACEL_API DelayValue
{
public:
	DelayValue() = default;

	DelayValue(float _timeUp, float _timeDown, float _timeReset)
		: m_reachTimeUp(_timeUp)
		, m_reachTimeDown(_timeDown)
		, m_reachTimeReset(_timeReset)
	{}

	inline void init(float _timeUp, float _timeDown, float _timeReset)
	{
		m_reachTimeUp = _timeUp;
		m_reachTimeDown = _timeDown;
		m_reachTimeReset = _timeReset;
	}

	float addValue(float _value, float _deltaTime);
	inline void reset()
	{
		m_value = 0.0f;
		m_valueTime = 0.0f;
		m_goal = 0.0f;
		m_start = 0.0f;
		m_duration = 0.0f;
		m_up = false;
	}

	~DelayValue() {};

private:
	float m_reachTimeUp { 0.0f };
	float m_reachTimeDown { 0.0f };
	float m_reachTimeReset { 0.0f };

	float m_value { 0.0f };
	float m_valueTime { 0.0f };
	float m_goal { 0.0f };
	float m_start { 0.0f };
	float m_duration { 0.0f };

	bool m_up { false };
};
