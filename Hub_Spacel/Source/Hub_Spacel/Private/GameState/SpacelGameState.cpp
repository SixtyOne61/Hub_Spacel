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

FString ASpacelGameState::GetBestTeam() const
{
    TOptional<int32> val {};
    FString teamName {};
    for (auto const& score : m_scores)
    {
        if (!val.IsSet() || score.Value > val.GetValue())
        {
            val = score.Value;
            teamName = score.Key;
        }
    }

    return teamName;
}

void ASpacelGameState::AddScore(FString const& _team, int32 _val)
{
    m_scores[_team] += _val;
}

void ASpacelGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASpacelGameState, R_LatestEvent);
    DOREPLIFETIME(ASpacelGameState, R_WinningTeam);
    DOREPLIFETIME(ASpacelGameState, RU_GameState);
}