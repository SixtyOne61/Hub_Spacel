// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCardWidget.h"
#include "Util/SimplyUI.h"
#include "Components/TextBlock.h"
#include "Player/SpacelPlayerState.h"

void UPlayerCardWidget::NativeConstruct()
{
    Super::NativeConstruct();

    PlayerNameTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_PlayerName"));
    AttackTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("Txt_Attack"));
    ProtectionTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("Txt_Protection"));
    SupportTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("Txt_Support"));
}
void UPlayerCardWidget::NativeDestruct()
{
    Super::NativeDestruct();
}

void UPlayerCardWidget::Setup(ASpacelPlayerState* _playerState)
{
    if(_playerState == nullptr) return;

    if (this->PlayerNameTextBlock != nullptr)
    {
        this->PlayerNameTextBlock->SetText(FText::FromString(_playerState->PlayerName));
    }

    if (this->AttackTextBlock != nullptr)
    {
        this->AttackTextBlock->SetText(FText::FromString(FString::FromInt(_playerState->Attack)));
    }

    if (this->ProtectionTextBlock != nullptr)
    {
        this->ProtectionTextBlock->SetText(FText::FromString(FString::FromInt(_playerState->Protection)));
    }

    if (this->SupportTextBlock != nullptr)
    {
        this->SupportTextBlock->SetText(FText::FromString(FString::FromInt(_playerState->Support)));
    }
}