// Fill out your copyright notice in the Description page of Project Settings.

#include "SkillItemWidget.h"
#include "GameState/SpacelGameState.h"
#include "Player/SpacelPlayerState.h"
#include "Player/ShipPawn.h"
#include "Kismet/GameplayStatics.h"

void USkillItemWidget::NativeConstruct()
{
    Super::NativeConstruct();
    ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld()));
    if (spacelGameState != nullptr)
    {
        spacelGameState->OnChangeStateDelegate.AddDynamic(this, &USkillItemWidget::OnChangeState);
    }
}

void USkillItemWidget::OnChangeState(EGameState _state)
{
    if (_state == EGameState::LockPrepare)
    {
        this->OnChooseSkillDelegate.Clear();
        this->OnHoverSkillDelegate.Clear();
        m_isDisable = true;
    }
    else if (_state == EGameState::InGame)
    {
        this->RemoveFromParent();
    }
}

void USkillItemWidget::OnChooseSkill()
{
    if(m_isDisable) return;

    if (ASpacelPlayerState* owningPlayerState = Cast<ASpacelPlayerState>(this->GetOwningPlayerState()))
    {
        owningPlayerState->RPCServerAddSkill(Data.Id, Data.Type);

        this->OnChooseSkillDelegate.Broadcast(Data.Id, Data.VerboseEffect, Data.Type, Data.Icon, Data.BackgroundColor);
    }
}

void USkillItemWidget::OnHover()
{
    if (m_isDisable) return;

    if (ASpacelPlayerState* owningPlayerState = Cast<ASpacelPlayerState>(this->GetOwningPlayerState()))
    {
        if (AShipPawn* shipPawn = Cast<AShipPawn>(owningPlayerState->GetPawn()))
        {
            shipPawn->buildLobbyShip(Data.Id, Data.Type);
        }
    }

    this->OnHoverSkillDelegate.Broadcast(Data.Type, Data.Title, Data.Desc, Data.BackgroundColor);
}

void USkillItemWidget::OnUnHover()
{
    if (ASpacelPlayerState* owningPlayerState = Cast<ASpacelPlayerState>(this->GetOwningPlayerState()))
    {
        if (AShipPawn* shipPawn = Cast<AShipPawn>(owningPlayerState->GetPawn()))
        {
            shipPawn->buildLobbyShip(ESkill::Max, ESkillType::Max);
        }
    }

    this->OnUnHoverSkillDelegate.Broadcast(Data.Type);
}