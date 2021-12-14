// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacelGameState.h"
#include "Player/SpacelPlayerState.h"
#include "Player/ShipPawn.h"
#include "Player/MetricComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "DataAsset/GameStateDataAsset.h"
#include "Hub_SpacelGameInstance.h"

void ASpacelGameState::travelToEndMenu()
{
    if (APlayerController* playerController = UGameplayStatics::GetPlayerController(this->GetWorld(), 0))
    {
        FString levelName{ "EndMenu" };
        playerController->ClientTravel(levelName, ETravelType::TRAVEL_Absolute);
    }
}

void ASpacelGameState::registerPlayerData()
{
    for (auto* playerState : this->PlayerArray)
    {
        if (ASpacelPlayerState const* spacelPlayerState = Cast<ASpacelPlayerState>(playerState))
        {
            this->R_PlayersData.Add({ spacelPlayerState->PlayerId,
                *spacelPlayerState->R_Team,
                *spacelPlayerState->GetPlayerName(),
                (ESkill)spacelPlayerState->R_LowSkill,
                (ESkill)spacelPlayerState->R_MediumSkill,
                (ESkill)spacelPlayerState->R_HightSkill });
        }
    }
}

void ASpacelGameState::sendMetric()
{
    if (auto* spacelGameInstance = this->GetGameInstance<UHub_SpacelGameInstance>())
    {
        for (auto const& data : R_PlayersData)
        {
            for (auto* player : this->PlayerArray)
            {
                if (player != nullptr)
                {
                    if (player->PlayerId == data.PlayerId)
                    {
                        if (AShipPawn* pawn = player->GetPawn<AShipPawn>())
                        {
                            if (UMetricComponent* component = Cast<UMetricComponent>(pawn->GetComponentByClass(UMetricComponent::StaticClass())))
                            {
                                auto killMetric = component->getMetric<SMetricIncrease>(EMetric::Kill);
                                auto deathMetric = component->getMetric<SMetricIncrease>(EMetric::Death);
                                auto assistMetric = component->getMetric<SMetricIncrease>(EMetric::Assist);
                                auto scoreMetric = component->getMetric<SMetricAdd>(EMetric::TotalScore);

                                auto precisionMetric = component->getMetric<SMetricRatio>(EMetric::Precision);
                                auto repairMetric = component->getMetric<SMetricAdd>(EMetric::MatiereUseForRepair);
                                auto healMetric = component->getMetric<SMetricAdd>(EMetric::Heal);
                                auto destructionMetric = component->getMetric<SMetricIncrease>(EMetric::Destruction);

                                spacelGameInstance->MetricPlayersData.Add({ data, 
                                    killMetric != nullptr ? std::get<0>(killMetric->get()) : 0,
                                    deathMetric != nullptr ? std::get<0>(deathMetric->get()) : 0,
                                    assistMetric != nullptr ? std::get<0>(assistMetric->get()) : 0,
                                    scoreMetric != nullptr ? std::get<0>(scoreMetric->get()) : 0,
                                    precisionMetric != nullptr ? (int)std::get<1>(precisionMetric->get()) : 0,
                                    repairMetric != nullptr ? std::get<0>(repairMetric->get()) : 0,
                                    healMetric != nullptr ? std::get<0>(healMetric->get()) : 0,
                                    destructionMetric != nullptr ? std::get<0>(destructionMetric->get()) : 0});
                            }
                        }
                    }
                }
            }
        }

        spacelGameInstance->ScoresData = R_Scores;
    }
}

void ASpacelGameState::OnRep_StateGame()
{
    OnChangeStateDelegate.Broadcast((EGameState)this->RU_GameState);

    if ((EGameState)this->RU_GameState == EGameState::WaitEnd &&
        this->GetNetMode() != ENetMode::NM_DedicatedServer)
    {
        travelToEndMenu();
    }
    else if ((EGameState)this->RU_GameState == EGameState::InGame &&
        this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        registerPlayerData();
    }
    else if ((EGameState)this->RU_GameState == EGameState::EndGame &&
        this->GetNetMode() == ENetMode::NM_Client)
    {
        sendMetric();
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

    // can be happear on end when stats doesn't complete yet
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
                    if (UMetricComponent* component = Cast<UMetricComponent>(shipPawn->GetComponentByClass(UMetricComponent::StaticClass())))
                    {
                        component->OnScored(_type, scoreValue);
                    }
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

void ASpacelGameState::AddScore(FString const& _team, EScoreType _type)
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

    // send feedback to all playerin team
    for (APlayerState* playerState : this->PlayerArray)
    {
        if (ASpacelPlayerState * spacelPlayerState = Cast<ASpacelPlayerState>(playerState))
        {
            if (spacelPlayerState->R_Team == _team)
            {
                if (AShipPawn* shipPawn = playerState->GetPawn<AShipPawn>())
                {
                    shipPawn->RPCClientFeedbackScore(_type, scoreValue);
                }
            }
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
    FMath::RandInit((int32)FDateTime::Now().GetMillisecond());
}

void ASpacelGameState::RPCNetMulticastStartGlobalCountDown_Implementation(int64 _syncPoint, uint16 _duration)
{
    int64 now = FDateTime::UtcNow().ToUnixTimestamp();
    // readjust location with sync time
    int64 deltaSecond = now - _syncPoint;

    this->GlobalSecondLocalCountDown = _duration - deltaSecond;

    OnStartLocalTimerDelegate.Broadcast(_duration - deltaSecond);
}

void ASpacelGameState::RPCNetMulticastStartMission_Implementation(EMission _type)
{
    this->R_MissionInProgress.Add({_type, ""});
    OnStartMissionDelegate.Broadcast(_type);
}

void ASpacelGameState::RPCNetMulticastStartMissionTwoParam_Implementation(EMission _type, FName _team, FName _teamTarget)
{
    this->R_MissionInProgress.Add({_type, _team});
    OnStartMissionTwoParamDelegate.Broadcast(_type, _team, _teamTarget);
}

void ASpacelGameState::RPCNetMulticastEndMission_Implementation(EMission _type, bool _succeed, FName _succeedForTeam)
{
    OnEndMissionDelegate.Broadcast(_type, _succeed, _succeedForTeam);
}

void ASpacelGameState::RPCNetMulticastResetTimerMission_Implementation(EMission _type)
{
    OnResetTimerMissionDelegate.Broadcast(_type);
}

void ASpacelGameState::RPCNetMulticastKill_Implementation(int32 _killer, int32 _killed)
{
    OnWhoKillWhoDelegate.Broadcast(_killer, _killed);
}

void ASpacelGameState::registerMission()
{
    if (this->GameStateDataAsset != nullptr)
    {
        RandomMissions = this->GameStateDataAsset->RandomMissions;

        for (auto timer : this->GameStateDataAsset->TimerMissions)
        {
            FTimerHandle handle;
            this->GetWorldTimerManager().SetTimer(handle, this, &ASpacelGameState::CallMission, 1.0f, false, timer);
        }
    }

    OnAskMissionDelegate.Broadcast(EMission::FirstBlood);
    OnAskMissionDelegate.Broadcast(EMission::ScoreRace);
}

void ASpacelGameState::CallMission()
{
    int32 id = FMath::RandRange(0, RandomMissions.Num() - 1);
    OnAskMissionDelegate.Broadcast(RandomMissions[id]);
    RandomMissions.RemoveAt(id);
}

void ASpacelGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASpacelGameState, R_LatestEvent);
    DOREPLIFETIME(ASpacelGameState, R_WinningTeam);
    DOREPLIFETIME(ASpacelGameState, RU_GameState);
    DOREPLIFETIME(ASpacelGameState, R_Scores);
    DOREPLIFETIME(ASpacelGameState, R_PlayersData);
    DOREPLIFETIME(ASpacelGameState, R_MinDeathTimer);
    DOREPLIFETIME(ASpacelGameState, R_MissionInProgress);
}