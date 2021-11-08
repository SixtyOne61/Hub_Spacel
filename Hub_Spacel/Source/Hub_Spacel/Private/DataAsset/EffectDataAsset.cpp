// Fill out your copyright notice in the Description page of Project Settings.


#include "EffectDataAsset.h"

FEffect const* UEffectDataAsset::getEffect(EEffect _type) const
{
    for (FEffect const& effect : this->Effects)
    {
        if (_type == effect.Type)
        {
            return &effect;
        }
    }

    ensure(false);
    return nullptr;
}