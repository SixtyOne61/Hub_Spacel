// Fill out your copyright notice in the Description page of Project Settings.

#include "PreparePhaseWidget.h"
#include "Components/TextBlock.h"
#include "Util/SimplyUI.h"
#include "Player/SpacelPlayerState.h"

void UPreparePhaseWidget::PostLoad()
{
    Super::PostLoad();

    ASpacelPlayerState* owningPlayerState{ Cast<ASpacelPlayerState>(this->GetOwningPlayerState()) };
    if (owningPlayerState != nullptr)
    {
        owningPlayerState->OnUpdateRemainingSkillPointDelegate.AddDynamic(this, &UPreparePhaseWidget::UpdateRemainingSkillPoint);
        this->UpdateRemainingSkillPoint();
    }
}

void UPreparePhaseWidget::NativeConstruct()
{
    Super::NativeConstruct();

    RemainingSkillPointTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_RemainingSkillPoint"));
    TimeTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_Time"));

    UWorld* world { this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    world->GetTimerManager().SetTimer(TimeHandle, this, &UPreparePhaseWidget::SetTime, 1.0f, true, 1.0f);

    if (!ensure(this->TimeTextBlock != nullptr)) return;
    this->TimeTextBlock->SetText(FText::FromString(FString::FromInt(this->RemainingTime)));
}

void UPreparePhaseWidget::UpdateRemainingSkillPoint()
{
    ASpacelPlayerState* owningPlayerState{ Cast<ASpacelPlayerState>(this->GetOwningPlayerState()) };
    if (owningPlayerState != nullptr)
    {
        if (!ensure(this->RemainingSkillPointTextBlock != nullptr)) return;
        this->RemainingSkillPointTextBlock->SetText(FText::FromString("Remaining Skill Point : " + FString::FromInt(owningPlayerState->getRemainingSkillPoint())));
    }
}

void UPreparePhaseWidget::SetTime()
{
    this->RemainingTime--;
    if (!ensure(this->TimeTextBlock != nullptr)) return;
    this->TimeTextBlock->SetText(FText::FromString(FString::FromInt(this->RemainingTime)));

    if (this->RemainingTime == 0)
    {
        this->OnStartGameDelegate.Broadcast();
        this->RemoveFromViewport();
    }
}