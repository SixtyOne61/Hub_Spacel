// Fill out your copyright notice in the Description page of Project Settings.


#include "VoxelAnimationComponent.h"
#include "DataAsset/CurveAnimationDataAsset.h"

UVoxelAnimationComponent::UVoxelAnimationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UVoxelAnimationComponent::start(TArray<FVector_NetQuantize>* _toAnim, EAnimationCurveType _type, bool _isUniform, FVector const& _delta, float _duration)
{
    m_anims.Add({ _type , _isUniform, _duration , 0.0f , _toAnim, _delta });
}

void UVoxelAnimationComponent::TickComponent(float _deltaSeconde, enum ELevelTick _tickType, FActorComponentTickFunction* _thisTickFunction)
{
    Super::TickComponent(_deltaSeconde, _tickType, _thisTickFunction);

    // clear anim
    m_anims.RemoveAll([](auto const& _anim) { return _anim.m_array == nullptr; });

    if(this->AnimationDataAsset == nullptr) return;

    for (auto& anim : m_anims)
    {
        // update timer
        anim.m_timer = FMath::Min(anim.m_timer + _deltaSeconde, anim.m_duration);

        anim.m_isUniform ? animUniform(anim, _deltaSeconde) : animUnUniform(anim, _deltaSeconde);

        // check end anim
        if (anim.m_timer >= anim.m_duration)
        {
            // unreference
            anim.m_array = nullptr;
        }
    }
}

void UVoxelAnimationComponent::animUniform(SAnim& _anim, float _deltaSeconde)
{
    if (FVoxelAnimationCurve* curve = this->AnimationDataAsset->get(_anim.m_type))
    {
        if (FRichCurve* richCurve = curve->Curve.GetRichCurve())
        {
            float curveValue = richCurve->Eval(_anim.m_timer / _anim.m_duration, 0.0f);

            // wrong formation
            FVector v0 = _anim.m_delta / _anim.m_duration;
            FVector vt { curveValue * _anim.m_timer + v0.X,
                            curveValue * _anim.m_timer + v0.Y,
                            curveValue * _anim.m_timer + v0.Z };

            FVector delta = (_anim.m_delta / vt) * _deltaSeconde;
            TArray<FVector_NetQuantize>& toAnim = *_anim.m_array;
            for (auto& location : toAnim)
            {
                location += delta;
            }
        }
    }

}

void UVoxelAnimationComponent::animUnUniform(SAnim& _anim, float _deltaSeconde)
{
}