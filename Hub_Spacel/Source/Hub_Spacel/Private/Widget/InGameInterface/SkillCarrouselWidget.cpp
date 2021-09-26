// Fill out your copyright notice in the Description page of Project Settings.


#include "SkillCarrouselWidget.h"
#include "DataAsset/SkillDataAsset.h"
#include "DataAsset/UniqueSkillDataAsset.h"
#include "Widget/InGameInterface/SkillItemWidget.h"

void USkillCarrouselWidget::SetupCarrousel(ESkillType _type)
{
    if(this->SkillDataAsset == nullptr) return;

    BP_CleanCarrousel();

    auto skills = this->SkillDataAsset->getSkillByType(_type);
    for (auto skill : skills)
    {
        if (skill != nullptr)
        {
            USkillItemWidget* widget { nullptr };
            BP_CreateSkillItemCarrousel(widget);
            if (widget != nullptr)
            {
                widget->setupItems({ skill->Skill, skill->BackgroundColorLobby, skill->Title, skill->Desc, skill->IconeBtn, skill->SkillType });
                widget->OnChooseSkillDelegate.AddDynamic(this, &USkillCarrouselWidget::OnChooseSkill);
                widget->OnHoverSkillDelegate.AddDynamic(this, &USkillCarrouselWidget::OnHoverSkill);
            }
        }
    }
}

void USkillCarrouselWidget::OnChooseSkill(ESkill _skillId, ESkillType _type)
{
    this->OnChangeCarrouselDelegate.Broadcast(_skillId, _type);
}

void USkillCarrouselWidget::OnHoverSkill(ESkill _skillId, ESkillType _type)
{
    this->OnHoverCarrouselDelegate.Broadcast(_skillId, _type);
}

