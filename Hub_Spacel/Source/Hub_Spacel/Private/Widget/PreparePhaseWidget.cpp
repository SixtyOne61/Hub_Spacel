// Fill out your copyright notice in the Description page of Project Settings.

#include "PreparePhaseWidget.h"
#include "Components/TextBlock.h"
#include "Util/SimplyUI.h"
#include "Player/SpacelPlayerState.h"
#include "GameState/SpacelGameState.h"
#include "Kismet/GameplayStatics.h"

void UPreparePhaseWidget::NativeConstruct()
{
    Super::NativeConstruct();
    bIsFocusable = true;

    RemainingSkillPointTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_RemainingSkillPoint"));
    TimeTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_Time"));

    if (!ensure(this->TimeTextBlock != nullptr)) return;
    this->TimeTextBlock->SetText(FText::FromString(FString::FromInt(this->RemainingTime)));

    ASpacelPlayerState* owningPlayerState{ Cast<ASpacelPlayerState>(this->GetOwningPlayerState()) };
    if (owningPlayerState != nullptr)
    {
        owningPlayerState->OnUpdateRemainingSkillPointDelegate.AddDynamic(this, &UPreparePhaseWidget::UpdateRemainingSkillPoint);
        this->UpdateRemainingSkillPoint();
    }

    ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld()));
    if (spacelGameState != nullptr)
    {
        spacelGameState->OnStartPrepareDelegate.AddDynamic(this, &UPreparePhaseWidget::StartPrepare);
        spacelGameState->OnStartGameDelegate.AddDynamic(this, &UPreparePhaseWidget::StartGame);
    }
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
        UWorld* world{ this->GetWorld() };
        if (!ensure(world != nullptr)) return;

        world->GetTimerManager().ClearTimer(this->TimeHandle);
    }
}

void UPreparePhaseWidget::StartPrepare()
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    world->GetTimerManager().SetTimer(TimeHandle, this, &UPreparePhaseWidget::SetTime, 1.0f, true, 0.0f);
}

void UPreparePhaseWidget::StartGame()
{
    this->RemoveFromViewport();

    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;
    world->GetTimerManager().ClearTimer(this->TimeHandle);
}