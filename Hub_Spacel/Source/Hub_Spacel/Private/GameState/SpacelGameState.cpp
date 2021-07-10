// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacelGameState.h"
#include "Player/SpacelPlayerState.h"
#include "Player/ShipPawn.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "DataAsset/GameStateDataAsset.h"

void ASpacelGameState::OnRep_StateGame()
{
    OnChangeStateDelegate.Broadcast((EGameState)this->RU_GameState);
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

FString ASpacelGameState::GetWorstTeam() const
{
    TOptional<int32> val{};
    FString teamName{};
    for (FScore const& score : this->R_Scores)
    {
        if (!val.IsSet() || score.Score < val.GetValue())
        {
            val = score.Score;
            teamName = score.Team;
        }
    }

    return teamName;
}

int32 ASpacelGameState::GetScore(FString const& _team) const
{
    for (FScore const& score : this->R_Scores)
    {
        if (score.Team == _team)
        {
            return score.Score;
        }
    }

    ensure(false);
    return 0;
}

void ASpacelGameState::AddScore(FString const& _team, int32 _playerId, EScoreType _type)
{
    if (this->GameStateDataAsset == nullptr) return;

    uint16 scoreValue = 0;
    for (FScore& score : this->R_Scores)
    {
        if (score.Team == _team)
        {
            int32 delta = this->GameStateDataAsset->getScore(_type);
            score.Score += delta;
            scoreValue = delta;
        }
    }

    // send feedback to player
    for (APlayerState* playerState : this->PlayerArray)
    {
        if (playerState != nullptr)
        {
            if (playerState->PlayerId == _playerId)
            {
                if (AShipPawn* shipPawn = playerState->GetPawn<AShipPawn>())
                {
                    shipPawn->RPCClientFeedbackScore(_type, scoreValue);
                }
            }
        }
    }
}

void ASpacelGameState::AddScore(FString const& _team, int32 _playerId, EScoreType _type, int32 _nb)
{
    for (int i = 0; i < _nb; ++i)
    {
        AddScore(_team, _playerId, _type);
    }
}

void ASpacelGameState::AddScore(FString const& _team, int32 _value)
{
    uint16 scoreValue = 0;
    for (FScore& score : this->R_Scores)
    {
        if (score.Team == _team)
        {
            score.Score += _value;
        }
    }
}

void ASpacelGameState::RegisterTeam()
{
    this->R_Scores.Empty();

    for (APlayerState* playerState : this->PlayerArray)
    {
        if (ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(playerState))
        {
            FString const& teamName = spacelPlayerState->R_Team;
            bool isNewTeam { true };
            for (FScore& score : this->R_Scores)
            {
                if (score.Team == teamName) isNewTeam = false;
            }

            if (isNewTeam)
            {
                // register team for scoring
                this->R_Scores.Add(FScore{ teamName , 0 });
            }
        }
    }
}

void ASpacelGameState::BeginPlay()
{
    Super::BeginPlay();
}

void ASpacelGameState::RPCNetMulticastStartMission_Implementation(EMission _type)
{
    OnStartMissionDelegate.Broadcast(_type);
}

void ASpacelGameState::RPCNetMulticastStartMissionTwoParam_Implementation(EMission _type, FName _team)
{
    OnStartMissionTwoParamDelegate.Broadcast(_type, _team);
}

void ASpacelGameState::RPCNetMulticastEndMission_Implementation(EMission _type)
{
    OnEndMissionDelegate.Broadcast(_type);
}

void ASpacelGameState::registerMission()
{
    if (this->GameStateDataAsset != nullptr)
    {
        FTimerHandle handle;
        this->GetWorldTimerManager().SetTimer(handle, this, &ASpacelGameState::FirstMission, 1.0f, false, this->GameStateDataAsset->TimerFirstMission);
    }

    OnAskMissionDelegate.Broadcast(EMission::FirstBlood);
    OnAskMissionDelegate.Broadcast(EMission::ScoreRace);
}

void ASpacelGameState::FirstMission()
{
    // TO DO
    OnAskMissionDelegate.Broadcast(EMission::Pirate);
}

void ASpacelGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASpacelGameState, R_LatestEvent);
    DOREPLIFETIME(ASpacelGameState, R_WinningTeam);
    DOREPLIFETIME(ASpacelGameState, RU_GameState);
    DOREPLIFETIME(ASpacelGameState, R_Scores);
}