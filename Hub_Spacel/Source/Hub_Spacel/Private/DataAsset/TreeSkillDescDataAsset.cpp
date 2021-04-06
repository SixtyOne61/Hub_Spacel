// Fill out your copyright notice in the Description page of Project Settings.


#include "TreeSkillDescDataAsset.h"

FDesc const& UTreeSkillDescDataAsset::getDesc(ESkillType _type) const
{
    for (FDesc const& desc : this->Descs)
    {
        if (desc.Type == _type)
        {
            return desc;
        }
    }

    ensure(false);
    static FDesc default = FDesc();
    return default;
}