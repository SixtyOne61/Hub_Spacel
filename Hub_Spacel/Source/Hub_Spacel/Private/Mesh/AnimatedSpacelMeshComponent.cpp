// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimatedSpacelMeshComponent.h"

UAnimatedSpacelMeshComponent::UAnimatedSpacelMeshComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UAnimatedSpacelMeshComponent::build()
{
    if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        Super::build();
    }
    else
    {
        m_timer = this->AnimationDuration;
    }
}

void UAnimatedSpacelMeshComponent::populate(EFormType _type, uint8 _ignoreLast, bool _useBonus)
{
    if (this->GetNetMode() != ENetMode::NM_DedicatedServer)
    {
        m_locationOnStart = this->Locations;
    }
    Super::populate(_type, _ignoreLast, _useBonus);
}

void UAnimatedSpacelMeshComponent::TickComponent(float _deltaTime, ELevelTick _tickType, FActorComponentTickFunction* _thisTickFunction)
{
    Super::TickComponent(_deltaTime, _tickType, _thisTickFunction);

    if (this->GetNetMode() != ENetMode::NM_DedicatedServer)
    {
        if (m_timer > 0.0f)
        {
            m_timer = FMath::Max(m_timer - _deltaTime, 0.0f);

            if (m_timer > 0.0f)
            {
                if (!localBuild())
                {
                    // reset timer, animation process failed
                    m_timer = 0.0f;
                    // ignore override build, timer has failed
                    Super::build();
                }
            }
            else
            {
                // ignore override build, timer has failedrain
                Super::build();
            }
        }
    }
}

bool UAnimatedSpacelMeshComponent::localBuild()
{
    int num = this->GetInstanceCount();

    /* check */
    if (num == m_locationOnStart.Num() && num == this->Locations.Num())
    {
        TArray<FTransform> trs{};
        for (int i = 0; i < num; ++i)
        {
            FTransform out{};
            this->GetInstanceTransform(i, out);

            FVector result = FMath::Lerp(this->Locations[i], m_locationOnStart[i], m_timer / this->AnimationDuration);
            out.SetLocation(result);

            // need to mark as dirty.
            this->UpdateInstanceTransform(i, out, false, true, false);
        }
        return true;
    }

    return false;
}