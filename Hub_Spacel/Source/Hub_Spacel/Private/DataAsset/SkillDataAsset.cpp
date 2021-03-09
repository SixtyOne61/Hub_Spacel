// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillDataAsset.h"

FSkill USkillDataAsset::getSKill(ESkill _type) const
{
    for (FSkill const& skill : this->Skills)
    {
        if (skill.Skill == _type)
        {
            return skill;
        }
    }

    ensure(false);
    return FSkill();
}