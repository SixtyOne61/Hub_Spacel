// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacelWidget.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"
#include "Components/ProgressBar.h"
#include "Player/SpacelPlayerState.h"
#include "Player/GamePlayerController.h"
#include "Player/ModuleComponent.h"
#include "Player/ShipPawn.h"
#include "GameState/SpacelGameState.h"
#include "Hub_SpacelGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Util/SimplyUI.h"
#include "DataAsset/PlayerDataAsset.h"

void USpacelWidget::NativeConstruct()
{
    Super::NativeConstruct();

    TeamNameTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_TeamName"));
    MatiereTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_Matiere"));
    TeammateCountTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_TeammateCount"));
    EventTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_Event"));
    PingTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_Ping"));
    ProtectionProgressBar = SimplyUI::initSafetyFromName<UUserWidget, UProgressBar>(this, TEXT("ProgressBar_Protection"));
    SupportProgressBar = SimplyUI::initSafetyFromName<UUserWidget, UProgressBar>(this, TEXT("ProgressBar_Support"));
    EscapeModeProgressBar = SimplyUI::initSafetyFromName<UUserWidget, UProgressBar>(this, TEXT("ProgressBar_EscapeMode"));

    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    world->GetTimerManager().SetTimer(SetTeammateCountHandle, this, &USpacelWidget::SetTeammateCount, 1.0f, true, 1.0f);
    world->GetTimerManager().SetTimer(SetLatestEventHandle, this, &USpacelWidget::SetLatestEvent, 1.0f, true, 1.0f);
    world->GetTimerManager().SetTimer(SetAverragePlayerLatencyHandle, this, &USpacelWidget::SetAverragePlayerLatency, 1.0f, true, 1.0f);

    this->SetVisibility(ESlateVisibility::Hidden);
    ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld()));
    if (spacelGameState != nullptr)
    {
        spacelGameState->OnStartGameDelegate.AddDynamic(this, &USpacelWidget::StartGame);
    }

    AShipPawn* shipPawn { this->GetOwningPlayerPawn<AShipPawn>() };
    if (shipPawn != nullptr)
    {
        shipPawn->OnEndUpdateMatiereDelegate.AddDynamic(this, &USpacelWidget::OnUpdateMatiere);
        shipPawn->OnStateEspaceModeChangeDelegate.AddDynamic(this, &USpacelWidget::OnChangeStateEscapeMode);

        shipPawn->ModuleComponent->OnUpdateCountProtectionDelegate.AddDynamic(this, &USpacelWidget::OnUpdateCountProtection);
        shipPawn->ModuleComponent->OnUpdateCountSupportDelegate.AddDynamic(this, &USpacelWidget::OnUpdateCountSupport);
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

void USpacelWidget::NativeTick(const FGeometry& _myGeometry, float _deltaTime)
{
    Super::NativeTick(_myGeometry, _deltaTime);

    if (AShipPawn* shipPawn = this->GetOwningPlayerPawn<AShipPawn>())
    {
        if (shipPawn->PlayerDataAsset != nullptr)
        {
            m_duration += _deltaTime;
            if (m_escapeMode == EEscapeMode::StateEscape)
            {
                updatePercent(this->EscapeModeProgressBar, m_duration / shipPawn->PlayerDataAsset->EscapeModeDuration);
            }
            else if (m_escapeMode == EEscapeMode::StateCountDown)
            {
                updatePercent(this->EscapeModeProgressBar, 1.0f - m_duration / shipPawn->PlayerDataAsset->EscapeModeCountDown);
            }
        }
    }
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

    FString latestEvent { spacelGameState->R_LatestEvent };

    if (latestEvent.Len() > 0)
    {
        if (latestEvent.Equals("GameEnded"))
        {
            FString winningTeam { spacelGameState->R_WinningTeam };

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

void USpacelWidget::OnUpdateMatiere(int32 _value)
{
    if (this->MatiereTextBlock != nullptr)
    {
        this->MatiereTextBlock->SetText(FText::FromString(FString::FromInt(_value)));
    }
}

void USpacelWidget::OnChangeStateEscapeMode(EEscapeMode _state)
{
    m_duration = 0.0f;
    m_escapeMode = _state;

    switch (_state)
    {
        case EEscapeMode::StateAvailable:
        case EEscapeMode::StateEscape:
            updatePercent(this->EscapeModeProgressBar, 0.0f);
        break;
        case EEscapeMode::StateCountDown:
            updatePercent(this->EscapeModeProgressBar, 1.0f);
        break;
    }
}

void USpacelWidget::OnUpdateCountProtection(int32 _value, int32 _max)
{
    if (_max > 0)
    {
        updatePercent(this->ProtectionProgressBar, (float)_value / (float)_max);
    }
}

void USpacelWidget::OnUpdateCountSupport(int32 _value, int32 _max)
{
    if (_max > 0)
    {
        updatePercent(this->SupportProgressBar, (float)_value / (float)_max);
    }
}

void USpacelWidget::updatePercent(UProgressBar* _progressBar, float _percent)
{
    if (_progressBar != nullptr)
    {
        _progressBar->SetPercent(_percent);
    }
}