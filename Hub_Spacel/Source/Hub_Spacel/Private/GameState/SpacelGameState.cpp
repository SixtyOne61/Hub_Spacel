// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacelGameState.h"
#include "Net/UnrealNetwork.h"

void ASpacelGameState::OnRep_StateGame()
{
    switch (this->RU_GameState)
    {
    case (uint8)EGameState::Prepare:
        OnStartPrepareDelegate.Broadcast();
        break;

    case (uint8)EGameState::InGame:
        OnStartGameDelegate.Broadcast();
        break;

    default:
        break;
    }
}

void ASpacelGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASpacelGameState, R_LatestEvent);
    DOREPLIFETIME(ASpacelGameState, R_WinningTeam);
    DOREPLIFETIME(ASpacelGameState, RU_GameState);
}