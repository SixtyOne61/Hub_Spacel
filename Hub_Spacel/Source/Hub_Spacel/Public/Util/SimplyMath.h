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

    inline FRotator MyLookRotation(FVector const& lookAt, FVector const& upDirection, FVector const& actorLocation)
    {
        FVector forward = lookAt - actorLocation;
        FVector up = upDirection;


        forward = forward.GetSafeNormal();
        up = up - (forward * FVector::DotProduct(up, forward));
        up = up.GetSafeNormal();

        ///////////////////////


        FVector vector = forward.GetSafeNormal();
        FVector vector2 = FVector::CrossProduct(up, vector);
        FVector vector3 = FVector::CrossProduct(vector, vector2);
        float m00 = vector.X;
        float m01 = vector.Y;
        float m02 = vector.Z;
        float m10 = vector2.X;
        float m11 = vector2.Y;
        float m12 = vector2.Z;
        float m20 = vector3.X;
        float m21 = vector3.Y;
        float m22 = vector3.Z;

        float num8 = (m00 + m11) + m22;
        FQuat quaternion = FQuat();

        if (num8 > 0.0f)
        {
            float num = (float)FMath::Sqrt(num8 + 1.0f);
            quaternion.W = num * 0.5f;
            num = 0.5f / num;
            quaternion.X = (m12 - m21) * num;
            quaternion.Y = (m20 - m02) * num;
            quaternion.Z = (m01 - m10) * num;
            return FRotator(quaternion);
        }

        if ((m00 >= m11) && (m00 >= m22))
        {
            float num7 = (float)FMath::Sqrt(((1.0f + m00) - m11) - m22);
            float num4 = 0.5f / num7;
            quaternion.X = 0.5f * num7;
            quaternion.Y = (m01 + m10) * num4;
            quaternion.Z = (m02 + m20) * num4;
            quaternion.W = (m12 - m21) * num4;
            return FRotator(quaternion);
        }

        if (m11 > m22)
        {
            float num6 = (float)FMath::Sqrt(((1.0f + m11) - m00) - m22);
            float num3 = 0.5f / num6;
            quaternion.X = (m10 + m01) * num3;
            quaternion.Y = 0.5f * num6;
            quaternion.Z = (m21 + m12) * num3;
            quaternion.W = (m20 - m02) * num3;
            return FRotator(quaternion);
        }

        float num5 = (float)FMath::Sqrt(((1.0f + m22) - m00) - m11);
        float num2 = 0.5f / num5;
        quaternion.X = (m20 + m02) * num2;
        quaternion.Y = (m21 + m12) * num2;
        quaternion.Z = 0.5f * num5;
        quaternion.W = (m01 - m10) * num2;


        return FRotator(quaternion);
    }
}