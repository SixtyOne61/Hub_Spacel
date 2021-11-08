// Fill out your copyright notice in the Description page of Project Settings.

#include "SkillItemWidget.h"
#include "Player/SpacelPlayerState.h"
#include "Player/ShipPawn.h"

void USkillItemWidget::setupItems(FData && _data)
{
    m_data = std::forward<FData>(_data);
    BP_Setup(m_data.m_backgroundColor, m_data.m_title, m_data.m_desc, m_data.m_icon);
}

void USkillItemWidget::OnChooseSkill()
{
    if (ASpacelPlayerState* owningPlayerState = Cast<ASpacelPlayerState>(this->GetOwningPlayerState()))
    {
        owningPlayerState->RPCServerAddSkill(m_data.m_id, m_data.m_type);

        this->OnChooseSkillDelegate.Broadcast(m_data.m_id, m_data.m_type);
    }
}

void USkillItemWidget::OnHover()
{
    if (ASpacelPlayerState* owningPlayerState = Cast<ASpacelPlayerState>(this->GetOwningPlayerState()))
    {
        if (AShipPawn* shipPawn = Cast<AShipPawn>(owningPlayerState->GetPawn()))
        {
            shipPawn->buildLobbyShip(m_data.m_id, m_data.m_type);
        }
    }

    this->OnHoverSkillDelegate.Broadcast(m_data.m_id, m_data.m_type);
}