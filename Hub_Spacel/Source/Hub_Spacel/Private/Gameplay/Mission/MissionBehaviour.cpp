// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionBehaviour.h"
#include "GameState/SpacelGameState.h"
#include "Player/SpacelPlayerState.h"
#include "Player/ShipPawn.h"

void MissionFirstBlood::tick(float _deltaTime, UWorld* _world)
{
    if(_world == nullptr) return;
    if(m_isEnd) return;

    if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(_world->GetGameState()))
    {
        if (spacelGameState->GetState() == EGameState::UnlockMedium)
        {
            end(_world);
        }
    }
}

void MissionRaceScore::tick(float _deltaTime, UWorld* _world)
{
    if (_world == nullptr) return;
    if (m_isEnd) return;

    if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(_world->GetGameState()))
    {
        FString const& bestTeam = spacelGameState->GetBestTeam();
        int32 score = spacelGameState->GetScore(bestTeam);

        if (score >= m_mission.RewardValue)
        {
            spacelGameState->GoToUnlockUltimate();
            end(_world);
        }
    }
}

void MissionEcartType::tick(float _deltaTime, UWorld* _world)
{
    if (m_killDone)
    {
        if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(_world->GetGameState()))
        {
            spacelGameState->AddScore(m_loosingTeam, m_mission.RewardValue);
        }
        end(_world);
    }
    else if(!m_start)
    {
        if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(_world->GetGameState()))
        {
            FString const& bestTeam = spacelGameState->GetBestTeam();
            FString const& worstTeam = spacelGameState->GetWorstTeam();

            if(bestTeam.IsEmpty() || worstTeam.IsEmpty()) return;

            int32 bestTeamScore = spacelGameState->GetScore(bestTeam);
            int32 worstTeamScore = spacelGameState->GetScore(worstTeam);

            if (bestTeamScore - worstTeamScore > m_mission.ConditionValue)
            {
                m_start = true;
                m_mission.Team = bestTeam;
                m_loosingTeam = worstTeam;

                // find player of this team
                TArray<APlayerState*> playerStates = spacelGameState->PlayerArray;
                for (auto* playerState : playerStates)
                {
                    if (AShipPawn* shipPawn = playerState->GetPawn<AShipPawn>())
                    {
                        if (shipPawn->Team == *worstTeam)
                        {
                            shipPawn->RPCClientStartMission(m_mission);
                        }
                        else
                        {
                            shipPawn->OnKill.add(std::bind(&MissionEcartType::onKill, this, std::placeholders::_1, std::placeholders::_2));
                        }
                    }
                }
            }
        }
    }
    else
    {
        m_timer += _deltaTime;
        if (m_timer >= m_mission.DurationValue)
        {
            end(_world);
        }
    }
}

void MissionEcartType::onKill(FString const& _victim, FString const& _killer)
{
    if (_victim == m_mission.Team && _killer == m_loosingTeam)
    {
        m_killDone = true;
    }
}