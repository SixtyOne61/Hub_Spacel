// Fill out your copyright notice in the Description page of Project Settings.

#include "SelectorSkillWidget.h"
#include "Util/SimplyUI.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Player/SpacelPlayerState.h"
#include "Widget/PreparePhaseWidget.h"

USelectorSkillWidget::USelectorSkillWidget(FObjectInitializer const& _objectInitializer)
    : Super(_objectInitializer)
{
}

void USelectorSkillWidget::NativeConstruct()
{
    Super::NativeConstruct();

    bIsFocusable = true;
    MinButton = SimplyUI::initSafetyFromName<UUserWidget, UButton>(this, TEXT("MinBtn"));
    FScriptDelegate minDelegate {};
    minDelegate.BindUFunction(this, "OnMinButtonClicked");
    MinButton->OnClicked.Add(minDelegate);

    MaxButton = SimplyUI::initSafetyFromName<UUserWidget, UButton>(this, TEXT("MaxBtn"));
    FScriptDelegate maxDelegate{};
    maxDelegate.BindUFunction(this, "OnMaxButtonClicked");
    MaxButton->OnClicked.Add(maxDelegate);

    Images[0] = SimplyUI::initSafetyFromName<UUserWidget, UImage>(this, TEXT("Level1"));
    Images[1] = SimplyUI::initSafetyFromName<UUserWidget, UImage>(this, TEXT("Level2"));
    Images[2] = SimplyUI::initSafetyFromName<UUserWidget, UImage>(this, TEXT("Level3"));
}

void USelectorSkillWidget::OnMinButtonClicked()
{
    ASpacelPlayerState* owningPlayerState{ Cast<ASpacelPlayerState>(this->GetOwningPlayerState()) };
    if (owningPlayerState != nullptr)
    {
        if (owningPlayerState->m_skillPoints.Contains(this->SkillType) 
            && owningPlayerState->m_skillPoints[this->SkillType] > 0)
        {
            uint8 num = owningPlayerState->m_skillPoints[this->SkillType] - 1;
            if (num < Images.Num())
            {
                this->Images[num]->SetBrushTintColor(this->OffColor);
            }
            owningPlayerState->m_skillPoints[this->SkillType]--;
            owningPlayerState->setRemainingSkillPoint(owningPlayerState->RemainingSkillPoint + 1);
        }
    }
}

void USelectorSkillWidget::OnMaxButtonClicked()
{
    ASpacelPlayerState* owningPlayerState{ Cast<ASpacelPlayerState>(this->GetOwningPlayerState()) };
    if (owningPlayerState != nullptr)
    {
        if (owningPlayerState->RemainingSkillPoint > 0
            && owningPlayerState->m_skillPoints.Contains(this->SkillType)
            && owningPlayerState->m_skillPoints[this->SkillType] < 3)
        {
            uint8 num = owningPlayerState->m_skillPoints[this->SkillType];
            if (num < Images.Num())
            {
                this->Images[num]->SetBrushTintColor(this->OnColor);
            }
            owningPlayerState->m_skillPoints[this->SkillType]++;
            owningPlayerState->setRemainingSkillPoint(owningPlayerState->RemainingSkillPoint - 1);
        }
    }
}