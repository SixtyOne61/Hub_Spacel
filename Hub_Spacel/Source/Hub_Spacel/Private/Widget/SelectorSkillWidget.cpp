// Fill out your copyright notice in the Description page of Project Settings.

#include "SelectorSkillWidget.h"
#include "Util/SimplyUI.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
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

    TArray<FName> imgName{ TEXT("Level1Img"), TEXT("Level2Img"), TEXT("Level3Img") };
    SimplyUI::initArray(this, Images, imgName);
    TArray<FName> borderName{ TEXT("Level1Border"), TEXT("Level2Border"), TEXT("Level3Border") };
    SimplyUI::initArray(this, Borders, borderName);

    for (int i = 0; i < Images.Num(); ++i)
    {
        if (Images[i] != nullptr)
        {
            if (i < LevelTexture.Num() && LevelTexture[i] != nullptr)
            {
                Images[i]->SetBrushFromTexture(LevelTexture[i]);
            }
        }
    }

    updateBorderColor();

    Type = SimplyUI::initSafetyFromName<UUserWidget, UImage>(this, TEXT("TypeImg"));
    if (Type != nullptr && TypeTexture != nullptr)
    {
        Type->SetBrushFromTexture(TypeTexture);
    }

    PlusButton = SimplyUI::initSafetyFromName<UUserWidget, UButton>(this, TEXT("+Btn"));
    if (PlusButton != nullptr)
    {
        FScriptDelegate btnPlusDelegate{};
        btnPlusDelegate.BindUFunction(this, "OnPlus");
        PlusButton->OnClicked.Add(btnPlusDelegate);
    }

    MinusButton = SimplyUI::initSafetyFromName<UUserWidget, UButton>(this, TEXT("-Btn"));
    if (MinusButton != nullptr)
    {
        FScriptDelegate btnMinusDelegate{};
        btnMinusDelegate.BindUFunction(this, "OnMinus");
        MinusButton->OnClicked.Add(btnMinusDelegate);
    }

    PointText = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("SkillPointTxt"));

    SetTypeBackground(FSlateColor(BackgroundColor));
}

void USelectorSkillWidget::OnPlus()
{
    if (m_isLock) return;
    onLevel(1);
}

void USelectorSkillWidget::OnMinus()
{
    if (m_isLock) return;
    onLevel(-1);
}

void USelectorSkillWidget::onLevel(int8 _delta)
{
    /*ASpacelPlayerState* owningPlayerState = Cast<ASpacelPlayerState>(this->GetOwningPlayerState());
    if (owningPlayerState == nullptr) return;

    if (_delta < 0)
    {
        if (m_currentLevel > 0)
        {
            m_currentLevel--;
            uint8 remainingSkillPoint = owningPlayerState->getRemainingSkillPoint();
            owningPlayerState->setRemainingSkillPoint(remainingSkillPoint + 1);
            owningPlayerState->RPCSetSkillPoint(this->SkillType, m_currentLevel);
            updateBorderColor();
        }
    }
    else
    {
        if (m_currentLevel < ASpacelPlayerState::MaxSkillPointType)
        {
            uint8 remainingSkillPoint = owningPlayerState->getRemainingSkillPoint();
            if (remainingSkillPoint > 0)
            {
                ++m_currentLevel;
                owningPlayerState->setRemainingSkillPoint(remainingSkillPoint - 1);
                owningPlayerState->RPCSetSkillPoint(this->SkillType, m_currentLevel);
                updateBorderColor();
            }
        }
    }*/
}

void USelectorSkillWidget::updateBorderColor()
{
    for (int i = 0; i < this->Borders.Num(); ++i)
    {
        if (this->Borders[i] != nullptr)
        {
            this->Borders[i]->SetBrushColor(m_currentLevel > i ? this->LockColor : this->BackgroundColor);
        }
    }

    if (this->PointText != nullptr)
    {
        this->PointText->SetText(FText::FromString(FString::FromInt(m_currentLevel)));
    }

    this->OnUpdateSkillDelegate.Broadcast(this->SkillType, m_currentLevel);
}