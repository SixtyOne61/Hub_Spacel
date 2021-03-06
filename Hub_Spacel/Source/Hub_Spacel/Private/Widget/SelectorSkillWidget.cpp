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
        uint8 level = owningPlayerState->getSkillPoint(this->SkillType);
        if (level > 0)
        {
            level--;
            if (level < Images.Num())
            {
                this->Images[level]->SetBrushTintColor(this->OffColor);
            }
            owningPlayerState->RPCSetSkillPoint(this->SkillType, level);
            owningPlayerState->setRemainingSkillPoint(owningPlayerState->RemainingSkillPoint + 1);
        }
    }
}

void USelectorSkillWidget::OnMaxButtonClicked()
{
    ASpacelPlayerState* owningPlayerState{ Cast<ASpacelPlayerState>(this->GetOwningPlayerState()) };
    if (owningPlayerState != nullptr)
    {
        uint8 level = owningPlayerState->getSkillPoint(this->SkillType);
        if (owningPlayerState->RemainingSkillPoint > 0
            && level < 3)
        {
            if (level < Images.Num())
            {
                this->Images[level]->SetBrushTintColor(this->OnColor);
            }
            ++level;
            owningPlayerState->RPCSetSkillPoint(this->SkillType, level);
            owningPlayerState->setRemainingSkillPoint(owningPlayerState->RemainingSkillPoint - 1);
        }
    }
}