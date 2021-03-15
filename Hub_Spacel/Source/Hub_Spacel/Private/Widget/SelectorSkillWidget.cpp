// Fill out your copyright notice in the Description page of Project Settings.

#include "SelectorSkillWidget.h"
#include "Util/SimplyUI.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Player/SpacelPlayerState.h"
#include "Widget/PreparePhaseWidget.h"
#include "Framework/SlateDelegates.h"

USelectorSkillWidget::USelectorSkillWidget(FObjectInitializer const& _objectInitializer)
    : Super(_objectInitializer)
{
}

void USelectorSkillWidget::NativeConstruct()
{
    Super::NativeConstruct();

    bIsFocusable = true;

    TArray<FName> btnName {TEXT("Btn_Lvl1"), TEXT("Btn_Lvl2"), TEXT("Btn_Lvl3")};
    SimplyUI::initArray(this, Buttons, btnName);
    TArray<FName> imgName{ TEXT("Img_Lvl1"), TEXT("Img_Lvl2"), TEXT("Img_Lvl3") };
    SimplyUI::initArray(this, Images, imgName);

    for (int i{ 0 }; i < Buttons.Num(); ++i)
    {
        if (Buttons[i] == nullptr) continue;

        FScriptDelegate btnDelegate{};
        FString name = "OnLevel" + FString::FromInt(i+1);
        btnDelegate.BindUFunction(this, *name);
        Buttons[i]->OnClicked.Add(btnDelegate);
    }
}

void USelectorSkillWidget::OnLevel1()
{
    if(m_isLock) return;
    onLevel(0);
}

void USelectorSkillWidget::OnLevel2()
{
    if (m_isLock) return;
    onLevel(1);
}

void USelectorSkillWidget::OnLevel3()
{
    if (m_isLock) return;
    onLevel(2);
}

void USelectorSkillWidget::onLevel(uint8 _level)
{
    for (int i{ 0 }; i < this->Images.Num(); ++i)
    {
        if(this->Images[i] == nullptr) continue;

        this->Images[i]->SetBrushTintColor(i <= _level ? this->OnColor : this->OffColor);
    }

    this->OnClickLevelDelegate.Broadcast(this->SkillType, _level+1);
}

void USelectorSkillWidget::reset()
{
    for (int i{ 0 }; i < this->Images.Num(); ++i)
    {
        if (this->Images[i] == nullptr) continue;

        this->Images[i]->SetBrushTintColor(this->OffColor);
    }
}
