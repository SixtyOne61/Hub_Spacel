// Fill out your copyright notice in the Description page of Project Settings.


#include "DelayValue.h"
#include "Util/SimplyMath.h"

float DelayValue::addValue(float _value, float _deltaTime)
{
    auto lb_reset = [&](float _duration, float _goal, bool _up)
    {
        m_valueTime = 0.0f;
        m_duration = _duration;
        m_goal = _goal;
        m_start = m_value;
        m_up = _up;
    };

    if (_value == m_value)
    {
        return m_value;
    }

    if (FMath::IsNearlyZero(_value) && !FMath::IsNearlyZero(m_value))
    {
        lb_reset(m_reachTimeReset, 0.0f, true);
    }
    else if (SimplyMath::IsSignDiff<float>(_value, m_value)
        || (m_value == 0.0f && _value != 0.0f))
    {
        //UE_LOG(LogTemp, Warning, TEXT("Reset"));
        if (_value > 0)
        {
            lb_reset(m_reachTimeUp, 1.0f, true);
        }
        else
        {
            lb_reset(m_reachTimeDown, -1.0f, true);
        }
    }

    m_valueTime += _deltaTime;
    float delta = FMath::Clamp(m_valueTime / m_duration, 0.0f, 1.0f);
    if (m_up)
    {
        m_value = FMath::Lerp(m_start, m_goal, delta);
        //m_value = SimplyMath::SinLerp(m_start, m_goal, delta);
    }
    else
    {
        m_value = FMath::Lerp(m_goal, m_start, delta);
        //m_value = SimplyMath::SinLerp(m_goal, m_start, delta);
    }

    return m_value;
}