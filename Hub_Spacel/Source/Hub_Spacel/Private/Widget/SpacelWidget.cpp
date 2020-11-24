// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacelWidget.h"
#include "Components/TextBlock.h"
#include "Player/SpacelPlayerState.h"
#include "GameState/SpacelGameState.h"
#include "GameMode/FlyingGameMode.h"
#include "Hub_SpacelGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Util/SimplyUI.h"
#include "Player/ShipPawn.h"

void USpacelWidget::NativeConstruct()
{
    Super::NativeConstruct();

    TeamNameTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_TeamName"));
    TeammateCountTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_TeammateCount"));
    EventTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_Event"));
    PingTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_Ping"));
    SpeedTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_Speed"));

    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    world->GetTimerManager().SetTimer(SetTeammateCountHandle, this, &USpacelWidget::SetTeammateCount, 1.0f, true, 1.0f);
    world->GetTimerManager().SetTimer(SetLatestEventHandle, this, &USpacelWidget::SetLatestEvent, 1.0f, true, 1.0f);
    world->GetTimerManager().SetTimer(SetAverragePlayerLatencyHandle, this, &USpacelWidget::SetAverragePlayerLatency, 1.0f, true, 1.0f);
    world->GetTimerManager().SetTimer(SetSpeedHandle, this, &USpacelWidget::SetSpeed, 0.2f, true, 1.0f);

    this->SetVisibility(ESlateVisibility::Hidden);
    AFlyingGameMode* flyingGameMode{ Cast<AFlyingGameMode>(UGameplayStatics::GetGameMode(this->GetWorld())) };
    if (flyingGameMode != nullptr)
    {
        flyingGameMode->OnStartGameDelegate.AddDynamic(this, &USpacelWidget::StartGame);
    }
}

void USpacelWidget::NativeDestruct()
{
    UWorld* world{ this->GetWorld() };
    if (world)
    {
        world->GetTimerManager().ClearTimer(SetTeammateCountHandle);
        world->GetTimerManager().ClearTimer(SetLatestEventHandle);
        world->GetTimerManager().ClearTimer(SetAverragePlayerLatencyHandle);
        world->GetTimerManager().ClearTimer(SetSpeedHandle);
    }
    Super::NativeDestruct();
}

void USpacelWidget::StartGame()
{
    this->SetVisibility(ESlateVisibility::Visible);
}

void USpacelWidget::SetTeammateCount()
{
    // TO DO : optimize this, did we need to call this every second ??? Team name never change
    ASpacelPlayerState* owningPlayerState { Cast<ASpacelPlayerState>(this->GetOwningPlayerState()) };
    if (owningPlayerState == nullptr)
    {
        return;
    }

    FString owningPlayerTeam { owningPlayerState->Team };
    if (this->TeamNameTextBlock)
    {
        this->TeamNameTextBlock->SetText(FText::FromString("Team Name: " + owningPlayerTeam));
    }

    if (owningPlayerTeam.Len() > 0)
    {
        UWorld* world { this->GetWorld() };
        if (!ensure(world != nullptr)) return;
        if (!ensure(world->GetGameState() != nullptr)) return;

        int teammateCount { 0 };

        TArray<APlayerState*> const& playerStates { world->GetGameState()->PlayerArray };
        for (APlayerState* playerState : playerStates)
        {
            ASpacelPlayerState* spacelPlayerState { Cast<ASpacelPlayerState>(playerState) };
            if (spacelPlayerState != nullptr && spacelPlayerState->Team.Equals(owningPlayerTeam))
            {
                teammateCount++;
            }
        }

        if (this->TeammateCountTextBlock)
        {
            this->TeammateCountTextBlock->SetText(FText::FromString("Teammate Count: " + FString::FromInt(teammateCount)));
        }
    }
}

void USpacelWidget::SetLatestEvent()
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    ASpacelGameState* spacelGameState { Cast<ASpacelGameState>(world->GetGameState()) };
    if (!ensure(spacelGameState != nullptr)) return;

    FString latestEvent { spacelGameState->LatestEvent };

    if (latestEvent.Len() > 0)
    {
        if (latestEvent.Equals("GameEnded"))
        {
            FString winningTeam { spacelGameState->WinningTeam };

            if (this->EventTextBlock)
            {
                this->EventTextBlock->SetText(FText::FromString(winningTeam + " won!"));
            }
        }
        else
        {
            if (this->EventTextBlock)
            {
                this->EventTextBlock->SetText(FText::FromString(latestEvent));
            }
        }
    }
}

void USpacelWidget::SetAverragePlayerLatency()
{
    UHub_SpacelGameInstance* spacelGameInstance{ Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
    if (!ensure(spacelGameInstance != nullptr)) return;

    float totalPlayerLatency{ 0.0f };
    for (float playerLatency : spacelGameInstance->PlayerLatencies)
    {
        totalPlayerLatency += playerLatency;
    }

    int32 num = spacelGameInstance->PlayerLatencies.Num();
    if (totalPlayerLatency > 0.0f && num != 0)
    {
        float averagePlayerLatency { totalPlayerLatency / num };
        FString pingString{ "Ping: " + FString::FromInt(FMath::RoundToInt(averagePlayerLatency)) + "ms" };
        if (!ensure(this->PingTextBlock != nullptr)) return;
        this->PingTextBlock->SetText(FText::FromString(pingString));
    }
}

void USpacelWidget::SetSpeed()
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    AShipPawn* shipPawn = Cast<AShipPawn>(UGameplayStatics::GetPlayerPawn(world, 0));
    if (shipPawn != nullptr && this->SpeedTextBlock != nullptr)
    {
        this->SpeedTextBlock->SetText(FText::FromString(FString::FromInt(FMath::RoundToInt(shipPawn->PercentSpeed * 100)) + " %"));
    }
}
