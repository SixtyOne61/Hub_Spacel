// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacelGameState.h"
#include "Player/SpacelPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"

void ASpacelGameState::OnRep_StateGame()
{
    switch (this->RU_GameState)
    {
    case (uint8)EGameState::Prepare:
        OnStartPrepareDelegate.Broadcast();
        break;

    case (uint8)EGameState::LockPrepare:
        OnLockPrepareDelegate.Broadcast();
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
    for (FScore const& score : this->R_Scores)
    {
        if (!val.IsSet() || score.Score > val.GetValue())
        {
            val = score.Score;
            teamName = score.Team;
        }
    }

    return teamName;
}

void ASpacelGameState::AddScore(FString const& _team, EScoreType _type)
{
    for (FScore & score : this->R_Scores)
    {
        if (score.Team == _team)
        {
            switch (_type)
            {
            case EScoreType::Hit:
                score.Score += 5;
                break;

            case EScoreType::Kill:
                score.Score += 300;
                break;

            case EScoreType::Tank:
                score.Score += 10;
                break;
            }
        }
    }
}

void ASpacelGameState::AddScore(FString const& _team, EScoreType _type, int32 _nb)
{
    for (int i = 0; i < _nb; ++i)
    {
        AddScore(_team, _type);
    }
}

void ASpacelGameState::RegisterTeam()
{
    this->R_Scores.Empty();

    for (APlayerState* playerState : this->PlayerArray)
    {
        if (ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(playerState))
        {
            FString const& teamName = spacelPlayerState->Team;
            for (FScore& score : this->R_Scores)
            {
                if (score.Team == teamName) break;
            }

            // register team for scoring
            this->R_Scores.Add(FScore { teamName , 0});
        }
    }
}

void ASpacelGameState::BeginPlay()
{
    Super::BeginPlay();
}

void ASpacelGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASpacelGameState, R_LatestEvent);
    DOREPLIFETIME(ASpacelGameState, R_WinningTeam);
    DOREPLIFETIME(ASpacelGameState, RU_GameState);
    DOREPLIFETIME(ASpacelGameState, R_Scores);
}