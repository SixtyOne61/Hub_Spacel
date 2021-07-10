// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionBehaviour.h"
#include "GameState/SpacelGameState.h"
#include "Gameplay/Mission/Pirate.h"
#include "Player/SpacelPlayerState.h"
#include "Player/ShipPawn.h"
#include "DataAsset/ParamMissionDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Util/Tag.h"

void MissionFirstBlood::tick(float _deltaTime, UWorld* _world)
{
    MissionBehaviour::tick(_deltaTime, _world);
    if(_world == nullptr) return;

    if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(_world->GetGameState()))
    {
        if (spacelGameState->GetState() == EGameState::UnlockMedium)
        {
            end();
        }
    }
}

void MissionRaceScore::tick(float _deltaTime, UWorld* _world)
{
    MissionBehaviour::tick(_deltaTime, _world);
    if (_world == nullptr) return;

    if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(_world->GetGameState()))
    {
        FString const& bestTeam = spacelGameState->GetBestTeam();
        int32 score = spacelGameState->GetScore(bestTeam);

        if (score >= m_mission.RewardValue)
        {
            spacelGameState->GoToUnlockUltimate();
            end();
        }
    }
}

void MissionEcartType::start(UWorld* _world)
{
    if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(_world->GetGameState()))
    {
        FString const& bestTeam = spacelGameState->GetBestTeam();
        FString const& worstTeam = spacelGameState->GetWorstTeam();

        if (bestTeam.IsEmpty() || worstTeam.IsEmpty()) return;

        int32 bestTeamScore = spacelGameState->GetScore(bestTeam);
        int32 worstTeamScore = spacelGameState->GetScore(worstTeam);

        if (bestTeamScore - worstTeamScore > m_mission.ConditionValue)
        {
            m_mission.Team = bestTeam;
            m_loosingTeam = worstTeam;

            spacelGameState->RPCNetMulticastStartMissionTwoParam(m_mission.Type, *worstTeam);

            // find player of this team
            TArray<APlayerState*> playerStates = spacelGameState->PlayerArray;
            for (auto* playerState : playerStates)
            {
                if (AShipPawn* shipPawn = playerState->GetPawn<AShipPawn>())
                {
                    if (shipPawn->Team != *worstTeam)
                    {
                        shipPawn->OnKill.add(std::bind(&MissionEcartType::onKill, this, std::placeholders::_1, std::placeholders::_2));
                    }
                }
            }
        }
    }
}

void MissionEcartType::tick(float _deltaTime, UWorld* _world)
{
    MissionSilence::tick(_deltaTime, _world);
    if (m_killDone)
    {
        if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(_world->GetGameState()))
        {
            spacelGameState->AddScore(m_loosingTeam, m_mission.RewardValue);
        }
        end();
    }

    m_timer += _deltaTime;
    if (m_timer >= m_mission.DurationValue)
    {
        end();
    }
}

void MissionEcartType::onKill(FString const& _victim, FString const& _killer)
{
    if (_victim == m_mission.Team && _killer == m_loosingTeam)
    {
        m_killDone = true;
    }
}

void MissionComet::tick(float _deltaTime, UWorld* _world)
{
    MissionBehaviour::tick(_deltaTime, _world);
    if (m_teams.Num() != 0)
    {
        if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(_world->GetGameState()))
        {
            TArray<APlayerState*> playerStates = spacelGameState->PlayerArray;
            for (auto* playerState : playerStates)
            {
                if (AShipPawn* shipPawn = playerState->GetPawn<AShipPawn>())
                {
                    for (auto const& team : m_teams)
                    {
                        if (shipPawn->Team == *team)
                        {
                            shipPawn->boostWall();
                        }
                    }
                }
            }
        }
        m_teams.Empty();
    }

    if (m_nbComet == 0)
    {
        end();
    }
}

void MissionComet::onCometDestroy(FString const& _team)
{
    m_nbComet--;
    m_teams.Add(_team);
}

void MissionPirate::start(class UWorld* _world)
{
    if (UParamPirateDataAsset* param = Cast<UParamPirateDataAsset>(m_mission.Param))
    {
        TArray<AActor*> out;
        UGameplayStatics::GetAllActorsWithTag(_world, Tags::Pirate, out);

        if (out.Num() != 0 && out[0] != nullptr)
        {
            FTransform transform = out[0]->GetTransform();
            if (AActor* actor = UGameplayStatics::BeginDeferredActorSpawnFromClass(_world, param->PirateClass, transform))
            {
                actor->InitialLifeSpan = m_mission.DurationValue;
                if (APirate* pirate = Cast<APirate>(actor))
                {
                    pirate->OnKilledUniqueDelegate = std::bind(&MissionPirate::onKill, this, std::placeholders::_1);
                }
                UGameplayStatics::FinishSpawningActor(actor, transform);
            }
        }
    }
}

void MissionPirate::tick(float _deltaTime, UWorld* _world)
{
    MissionBehaviour::tick(_deltaTime, _world);
    if (m_team != "")
    {
        if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(_world->GetGameState()))
        {
            TArray<APlayerState*> playerStates = spacelGameState->PlayerArray;
            for (auto* playerState : playerStates)
            {
                if (AShipPawn* shipPawn = playerState->GetPawn<AShipPawn>())
                {
                    if (shipPawn->Team == m_team)
                    {
                        shipPawn->boostPassive(m_mission.Type, m_mission.RewardValue);
                    }
                }
            }
        }
        end();
    }
}

void MissionPirate::onKill(FName const& _team)
{
    m_team = _team;
}