// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacelPlayerState.h"
#include "Net/UnrealNetwork.h"

void ASpacelPlayerState::setRemainingSkillPoint(uint8 && _val)
{
    this->RemainingSkillPoint = _val;
    this->OnUpdateRemainingSkillPointDelegate.Broadcast();
}

void ASpacelPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASpacelPlayerState, Team);
}
