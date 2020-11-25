// Fill out your copyright notice in the Description page of Project Settings.

#include "PreparePhaseWidget.h"
#include "Components/TextBlock.h"
#include "Util/SimplyUI.h"
#include "Player/SpacelPlayerState.h"
#include "GameState/SpacelGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Widget/PlayerCardWidget.h"

void UPreparePhaseWidget::NativeConstruct()
{
    Super::NativeConstruct();
    bIsFocusable = true;

    RemainingSkillPointTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_RemainingSkillPoint"));
    TimeTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_Time"));
    TeamNameTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_TeamName"));
    Player1 = SimplyUI::initSafetyFromName<UUserWidget, UPlayerCardWidget>(this, TEXT("WBP_Player1"));
    Player2 = SimplyUI::initSafetyFromName<UUserWidget, UPlayerCardWidget>(this, TEXT("WBP_Player2"));

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

    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    world->GetTimerManager().SetTimer(SetPlayerCardHandle, this, &UPreparePhaseWidget::SetPlayerCard, 1.0f, true, 1.0f);
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
    world->GetTimerManager().ClearTimer(this->SetPlayerCardHandle);
}

void UPreparePhaseWidget::NativeDestruct()
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;
    world->GetTimerManager().ClearTimer(this->TimeHandle);
    world->GetTimerManager().ClearTimer(this->SetPlayerCardHandle);

    Super::NativeDestruct();
}

void UPreparePhaseWidget::SetPlayerCard()
{
    ASpacelPlayerState* owningPlayerState{ Cast<ASpacelPlayerState>(this->GetOwningPlayerState()) };
    if (owningPlayerState == nullptr)
    {
        return;
    }

    FString owningPlayerTeam{ owningPlayerState->Team };
    if (this->TeamNameTextBlock)
    {
        this->TeamNameTextBlock->SetText(FText::FromString(owningPlayerTeam));
    }

    if (owningPlayerTeam.Len() > 0)
    {
        UWorld* world{ this->GetWorld() };
        if (!ensure(world != nullptr)) return;
        if (!ensure(world->GetGameState() != nullptr)) return;

        bool isFirst = true;
        TArray<APlayerState*> const& playerStates{ world->GetGameState()->PlayerArray };
        for (APlayerState* playerState : playerStates)
        {
            ASpacelPlayerState* spacelPlayerState{ Cast<ASpacelPlayerState>(playerState) };
            if (spacelPlayerState != nullptr && spacelPlayerState->Team.Equals(owningPlayerTeam))
            {
                // TO DO Better
                if (isFirst)
                {
                    this->Player1->SetPlayerName(spacelPlayerState->PlayerName);
                    isFirst = false;
                }
                else
                {
                    this->Player2->SetPlayerName(spacelPlayerState->PlayerName);
                }
            }
        }
    }
}