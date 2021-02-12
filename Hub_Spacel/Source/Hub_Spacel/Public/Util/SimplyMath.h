// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <type_traits>

namespace SimplyMath
{
    template<typename T>
    T InvLerp(T _a, T _b, float _value)
    {
        if (FMath::IsNearlyEqual(_a, _b))
        {
            if (_value < _a)
            {
                return _a;
            }
            else
            {
                return _b;
            }
        }
        else
        {
            return FMath::Min(FMath::Max(_a, (_value - _a) / (_b - _a)), _b);
        }
    }

    template<typename T>
    T Lerp(T _a, T _b, float _value)
    {
        if (FMath::IsNearlyEqual(_a, _b))
        {
            if (_value < _a)
            {
                return _a;
            }
            else
            {
                return _b;
            }
        }
        else
        {
            return FMath::Min(FMath::Max(_a, _value * (_b - _a) + _a), _b);
        }
    }

    template<typename T,
        std::enable_if_t<std::is_arithmetic<T>::value, bool> = true>
    inline bool IsSignDiff(T _a, T _b)
    {
        return (_a > 0 && _b < 0) || (_a < 0 && _b > 0);
    }

    template<typename T>
    struct SIsType;

    template<>
    struct SIsType<float>
    {
        static bool isNearlyZero(float _a) { return _a < 0.001f && _a > -0.001f; }
    };
}