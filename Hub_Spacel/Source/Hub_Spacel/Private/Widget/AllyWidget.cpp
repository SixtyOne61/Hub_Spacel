// Fill out your copyright notice in the Description page of Project Settings.


#include "AllyWidget.h"
#include "Util/SimplyUI.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Player/SpacelPlayerState.h"
#include "Player/ShipPawn.h"

void UAllyWidget::NativeConstruct()
{
    Super::NativeConstruct();

    NameTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("Txt_Name"));
    ProtectionProgressBar = SimplyUI::initSafetyFromName<UUserWidget, UProgressBar>(this, TEXT("ProgressBar_Protection"));
    SupportProgressBar = SimplyUI::initSafetyFromName<UUserWidget, UProgressBar>(this, TEXT("ProgressBar_Support"));
    DisconnectImage = SimplyUI::initSafetyFromName<UUserWidget, UImage>(this, TEXT("Image_Disconnect"));
}

void UAllyWidget::NativeDestruct()
{
    UWorld* world{ this->GetWorld() };
    if (world)
    {
        world->GetTimerManager().ClearTimer(this->WatcherHandle);
    }
    Super::NativeDestruct();
}

void UAllyWidget::updatePercent(class UProgressBar* _progressBar, float _value)
{
    if (_progressBar != nullptr)
    {
        _progressBar->SetPercent(_value);
    }
}

void UAllyWidget::setWatcher(class ASpacelPlayerState* _state)
{
    if (this->NameTextBlock != nullptr
        && _state != nullptr)
    {
        this->NameTextBlock->SetText(FText::FromString(_state->GetPlayerName()));
    }
    m_watcher = _state;

    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    world->GetTimerManager().SetTimer(this->WatcherHandle, this, &UAllyWidget::UpdateWatcher, 1.0f, true, 1.0f);
}

void UAllyWidget::UpdateWatcher()
{
    ESlateVisibility disconnect = ESlateVisibility::HitTestInvisible;
    if (m_watcher.IsValid())
    {
        if (AShipPawn* shipPawn = m_watcher.Get()->GetPawn<AShipPawn>())
        {
            updatePercent(this->ProtectionProgressBar, shipPawn->getPercentProtection());
            updatePercent(this->SupportProgressBar, shipPawn->getPercentSupport());
            disconnect = ESlateVisibility::Hidden;
        }
    }

    if (this->DisconnectImage != nullptr)
    {
        DisconnectImage->Visibility = disconnect;
    }
}