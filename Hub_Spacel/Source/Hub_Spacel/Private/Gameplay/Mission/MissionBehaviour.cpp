// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionBehaviour.h"
#include "GameState/SpacelGameState.h"

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
}
