// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCardWidget.h"
#include "Util/SimplyUI.h"
#include "Components/TextBlock.h"

void UPlayerCardWidget::SetPlayerCardName(FString const& _playerName)
{
    if (this->PlayerNameTextBlock != nullptr)
    {
        this->PlayerNameTextBlock->SetText(FText::FromString(_playerName));
    }
}

void UPlayerCardWidget::NativeConstruct()
{
    Super::NativeConstruct();

    PlayerNameTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_PlayerName"));
}
void UPlayerCardWidget::NativeDestruct()
{
    Super::NativeDestruct();
}