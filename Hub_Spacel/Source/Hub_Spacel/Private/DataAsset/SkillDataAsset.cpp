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

TArray<class UUniqueSkillDataAsset*> USkillDataAsset::getSkillByInput(EInput _input)
{
    TArray<class UUniqueSkillDataAsset*> skills {};

    for (UUniqueSkillDataAsset* skill : this->Skills)
    {
        if (skill && skill->InputType == _input)
        {
            skills.Add(skill);
        }
    }

    return skills;
}