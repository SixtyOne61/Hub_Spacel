// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillDataAsset.h"
#include "DataAsset/UniqueSkillDataAsset.h"

UUniqueSkillDataAsset * USkillDataAsset::getSKill(ESkill _type)
{
    for (UUniqueSkillDataAsset * skill : this->Skills)
    {
        if (skill->Skill == _type)
        {
            return skill;
        }
    }

    ensure(false);
    return nullptr;
}