// Fill out your copyright notice in the Description page of Project Settings.

#include "PreparePhaseWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Util/SimplyUI.h"
#include "Player/SpacelPlayerState.h"
#include "GameState/SpacelGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Widget/PlayerCardWidget.h"
#include "Widget/SelectorSKillWidget.h"
#include "DataAsset/TeamColorDataAsset.h"

void UPreparePhaseWidget::NativeConstruct()
{
    Super::NativeConstruct();
    bIsFocusable = true;

    RemainingSkillPointTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_RemainingSkillPoint"));
    TimeTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_Time"));

    TArray<FName> skillName { TEXT("SelectorAttack"), TEXT("SelectorProtection"), TEXT("SelectorSupport") };
    for (int i{ 0 }; i < skillName.Num(); ++i)
    {
        SelectorSkillWidget[i] = SimplyUI::initSafetyFromName<UUserWidget, USelectorSkillWidget>(this, skillName[i]);
        SelectorSkillWidget[i]->OnClickLevelDelegate.AddDynamic(this, &UPreparePhaseWidget::OnClickLevel);
    }

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

    SetupOwningTeam();
}

void UPreparePhaseWidget::UpdateRemainingSkillPoint()
{
    ASpacelPlayerState* owningPlayerState{ Cast<ASpacelPlayerState>(this->GetOwningPlayerState()) };
    if (owningPlayerState != nullptr)
    {
        if (!ensure(this->RemainingSkillPointTextBlock != nullptr)) return;
        this->RemainingSkillPointTextBlock->SetText(FText::FromString(FString::FromInt(owningPlayerState->getRemainingSkillPoint())));
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

void UPreparePhaseWidget::NativeDestruct()
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;
    world->GetTimerManager().ClearTimer(this->TimeHandle);

    Super::NativeDestruct();
}

void UPreparePhaseWidget::SetupOwningTeam()
{
    ASpacelPlayerState* owningPlayerState{ Cast<ASpacelPlayerState>(this->GetOwningPlayerState()) };
    if (owningPlayerState == nullptr)
    {
        UWorld* world{ this->GetWorld() };
        if (!ensure(world != nullptr)) return;

        FTimerHandle handle;
        world->GetTimerManager().SetTimer(handle, this, &UPreparePhaseWidget::SetupOwningTeam, 1.0f, false, 0.0f);
        return;
    }

    FString owningPlayerTeam{ owningPlayerState->Team };
    if (this->Colors != nullptr)
    {
        this->SetupOutline(this->Colors->GetColor(owningPlayerTeam));
    }
}

void UPreparePhaseWidget::OnClickLevel(ESkillType _type, uint8 _level)
{
    ASpacelPlayerState* owningPlayerState{ Cast<ASpacelPlayerState>(this->GetOwningPlayerState()) };
    if (owningPlayerState == nullptr) return;

    uint8 remainingSkillPoint = owningPlayerState->getRemainingSkillPoint();
    uint8 currentSkillPoint = owningPlayerState->getSkillPoint(_type);
    if(currentSkillPoint == _level) return;

    if (currentSkillPoint > _level)
    {
        owningPlayerState->setRemainingSkillPoint(remainingSkillPoint + currentSkillPoint - _level);
    }

    owningPlayerState->RPCSetSkillPoint(_type, _level);
    if (remainingSkillPoint < _level)
    {
        // reset other skill type
        for (USelectorSkillWidget* skill : this->SelectorSkillWidget)
        {
            if(skill == nullptr || skill->SkillType == _type) continue;
            skill->reset();
            owningPlayerState->RPCSetSkillPoint(skill->SkillType, 0);
        }

        owningPlayerState->setRemainingSkillPoint(4 - _level);
    }
    else
    {
        owningPlayerState->setRemainingSkillPoint(remainingSkillPoint - _level);
    }
}