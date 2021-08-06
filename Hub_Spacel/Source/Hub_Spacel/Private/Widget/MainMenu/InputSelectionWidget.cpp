// Fill out your copyright notice in the Description page of Project Settings.


#include "InputSelectionWidget.h"
#include "Components/VerticalBox.h"
#include "Widget/MainMenu/InputWidget.h"
#include "DataAsset/HideSkillDataAsset.h"
#include "DataAsset/SkillDataAsset.h"
#include "DataAsset/UniqueSkillDataAsset.h"
#include "DataAsset/KeyDataAsset.h"
#include "Util/SimplyUI.h"
#include "Enum/SpacelEnum.h"
#include "Kismet/GameplayStatics.h"

void UInputSelectionWidget::Setup()
{
    if (this->KeyDataAsset == nullptr) return;
    if (this->SkillDataAsset == nullptr) return;
    if (this->HideSkillDataAsset == nullptr) return;

    if (UVerticalBox* vertical = SimplyUI::initSafetyFromName<UUserWidget, UVerticalBox>(this, TEXT("VerticalBox_Input")))
    {
        TArray<UWidget*> children = vertical->GetAllChildren();

        for (auto child : children)
        {
            if (UInputWidget* inputWidget = Cast<UInputWidget>(child))
            {
                EInput inputType = inputWidget->Type;
                if (FHideSkill* hideSkill = this->HideSkillDataAsset->get(inputType))
                {
                    auto skills = this->SkillDataAsset->getSkillByInput(inputType);
                    TArray<UTexture2D*> icons {};

                    for (auto skill : skills)
                    {
                        if (skill != nullptr)
                        {
                            icons.Add(skill->IconeBtn);
                        }
                    }

                    inputWidget->KeyDataAsset = this->KeyDataAsset;
                    inputWidget->ActionName = hideSkill->ActionName;
                    inputWidget->Value = hideSkill->Value;
                    inputWidget->Key = hideSkill->Key;
                    inputWidget->BP_Setup(*hideSkill->Desc, icons, this->KeyDataAsset->get(hideSkill->Key), hideSkill->Key);
                    inputWidget->UpdateInputDelegate.AddDynamic(this, &UInputSelectionWidget::OnUpdateInput);
                }
            }
        }
    }
}

void UInputSelectionWidget::OnUpdateInput(EInput _input, FKey _key)
{
    if (this->SkillDataAsset == nullptr) return;
    if (this->HideSkillDataAsset == nullptr) return;

    if (FHideSkill* hideSkill = this->HideSkillDataAsset->get(_input))
    {
        hideSkill->Key = _key;

        auto skills = this->SkillDataAsset->getSkillByInput(_input);
        for (auto skill : skills)
        {
            if (skill != nullptr)
            {
                skill->Key = _key;
            }
        }
    }
}