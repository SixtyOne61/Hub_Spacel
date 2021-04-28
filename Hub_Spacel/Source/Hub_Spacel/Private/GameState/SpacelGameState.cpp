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
    switch (this->RU_GameState)
    {
    case (uint8)EGameState::Prepare:
        OnStartPrepareDelegate.Broadcast();
        break;

    case (uint8)EGameState::LockPrepare:
        OnLockPrepareDelegate.Broadcast();
        break;

    case (uint8)EGameState::InGame:
    {
        OnStartGameDelegate.Broadcast();
        FTimerDelegate timerCallbackFirstBlood;
        timerCallbackFirstBlood.BindLambda([&]() { 
            FMission mission;
            this->MissionDataAsset->fillMission(EMission::FirstBlood, mission);
            OnStartMissionDelegate.Broadcast(mission); });

        FTimerHandle handleFirstBlood;
        this->GetWorldTimerManager().SetTimer(handleFirstBlood, timerCallbackFirstBlood, 5.0f, false);

        FTimerDelegate timerCallbackScore;
        timerCallbackScore.BindLambda([&]() {
            FMission mission;
            this->MissionDataAsset->fillMission(EMission::ScoreRace, mission);
            OnStartMissionDelegate.Broadcast(mission); });

        FTimerHandle handleScore;
        this->GetWorldTimerManager().SetTimer(handleScore, timerCallbackScore, 30.0f, false);
        break;
    }

    case (uint8)EGameState::UnlockMedium:
    {
        FMission mission;
        this->MissionDataAsset->fillMission(EMission::FirstBlood, mission);
        OnEndMissionDelegate.Broadcast(mission);
        OnUnlockSkillDelegate.Broadcast((EGameState)this->RU_GameState);
        break;
    }

    case (uint8)EGameState::UnlockUltimate:
    {
        FMission mission;
        this->MissionDataAsset->fillMission(EMission::ScoreRace, mission);
        OnEndMissionDelegate.Broadcast(mission);
        OnUnlockSkillDelegate.Broadcast((EGameState)this->RU_GameState);
        break;
    }

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
    if (this->MissionDataAsset == nullptr) return;

    FMission mission{};
    this->MissionDataAsset->fillMission(EMission::ScoreRace, mission);
    int32 thresholdUltimate = mission.ConditionValue;

    uint16 scoreValue = 0;
    for (FScore& score : this->R_Scores)
    {
        if (score.Team == _team)
        {
            int32 delta = this->GameStateDataAsset->getScore(_type);
            score.Score += delta;
            scoreValue = delta;

            if (score.Score >= thresholdUltimate && RU_GameState == (uint8)EGameState::UnlockMedium)
            {
                GoToUnlockUltimate();
            }
        }
    }

    teamScoreBoost();

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

void ASpacelGameState::teamScoreBoost()
{
    FString const& bestTeam = GetBestTeam();
    FString const& worstTeam = GetWorstTeam();

    int32 bestTeamScore = GetScore(bestTeam);
    int32 worstTeamScore = GetScore(worstTeam);

    if (this->MissionDataAsset != nullptr)
    {
        FMission mission;
        this->MissionDataAsset->fillMission(EMission::EcartType, mission);

        if (bestTeamScore - worstTeamScore > mission.ConditionValue
            && !this->TeamWithBonusMission.Contains(worstTeam))
        {
            this->TeamWithBonusMission.Add(worstTeam);
            mission.Team = bestTeam;
            RPCNetMulticastScoreBoost(mission);
        }
    }
}

void ASpacelGameState::RPCNetMulticastScoreBoost_Implementation(FMission const& _mission)
{
    this->OnStartMissionDelegate.Broadcast(_mission);
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

void ASpacelGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASpacelGameState, R_LatestEvent);
    DOREPLIFETIME(ASpacelGameState, R_WinningTeam);
    DOREPLIFETIME(ASpacelGameState, RU_GameState);
    DOREPLIFETIME(ASpacelGameState, R_Scores);
}