// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerStatWidget.h"
#include "DataAsset/SkillDataAsset.h"
#include "DataAsset/UniqueSkillDataAsset.h"

UTexture2D* UPlayerStatWidget::GetLogo(ESkill _skill) const
{
    if (this->SkillDataAsset != nullptr)
    {
        if (auto* uniqueSkill = this->SkillDataAsset->getSKill(_skill))
        {
            return uniqueSkill->IconeBtn;
        }
    }

    return nullptr;
}