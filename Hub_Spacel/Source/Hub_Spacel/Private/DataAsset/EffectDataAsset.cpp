// Fill out your copyright notice in the Description page of Project Settings.


#include "EffectDataAsset.h"

FEffect UEffectDataAsset::getEffect(EEffect _type) const
{
    for (FEffect const& effect : this->Effects)
    {
        if (TOFLAG(effect.Effect) & TOFLAG(_type))
        {
            return effect;
        }
    }

    ensure(false);
    return FEffect();
}