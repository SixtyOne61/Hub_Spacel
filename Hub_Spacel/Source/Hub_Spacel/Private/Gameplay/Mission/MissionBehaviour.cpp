// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionBehaviour.h"
#include "GameState/SpacelGameState.h"
#include "Gameplay/Mission/Pirate.h"
#include "Gameplay/Mission/Comet.h"
#include "Gameplay/Mission/Gold.h"
#include "Player/SpacelPlayerState.h"
#include "Player/ShipPawn.h"
#include "DataAsset/ParamMissionDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Util/Tag.h"

void MissionFirstBlood::start(class UWorld* _world)
{
    MissionBehaviour::start(_world);

    if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(_world->GetGameState()))
    {
        TArray<APlayerState*> playerStates = spacelGameState->PlayerArray;
        for (auto* playerState : playerStates)
        {
            if (AShipPawn* shipPawn = playerState->GetPawn<AShipPawn>())
            {
                shipPawn->OnKill.add(std::bind(&MissionFirstBlood::onKill, this, std::placeholders::_1, std::placeholders::_2));
            }
        }
    }
}

void MissionFirstBlood::onKill(FString const& _victim, FString const& _killer)
{
    m_killDone = true;
}

void MissionFirstBlood::tick(float _deltaTime, UWorld* _world)
{
    MissionBehaviour::tick(_deltaTime, _world);

    if (m_killDone)
    {
        end(_world);
    }
}

void MissionFirstBlood::end(class UWorld* _world)
{
    MissionBehaviour::end(_world);

    if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(_world->GetGameState()))
    {
        TArray<APlayerState*> playerStates = spacelGameState->PlayerArray;
        for (auto* playerState : playerStates)
        {
            if (AShipPawn* shipPawn = playerState->GetPawn<AShipPawn>())
            {
                shipPawn->OnKill.clean();
            }
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
            end(_world);
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
            MissionSilence::start(_world);

            m_mission.Team = bestTeam;
            m_loosingTeam = worstTeam;

            spacelGameState->RPCNetMulticastStartMissionTwoParam(m_mission.Type, *worstTeam, *bestTeam);

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
        end(_world);
    }
}

void MissionEcartType::end(UWorld* _world)
{
    MissionSilence::end(_world);

    if(_world == nullptr) return;

    // clean event
    if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(_world->GetGameState()))
    {
        TArray<APlayerState*> playerStates = spacelGameState->PlayerArray;
        for (auto* playerState : playerStates)
        {
            if (AShipPawn* shipPawn = playerState->GetPawn<AShipPawn>())
            {
                shipPawn->OnKill.clean();
            }
        }
    }
}

void MissionEcartType::onKill(FString const& _victim, FString const& _killer)
{
    if (_killer == m_loosingTeam)
    {
        m_killDone = true;
    }
}

void MissionComet::start(class UWorld* _world)
{
    MissionBehaviour::start(_world);
    if (UParamCometDataAsset* param = Cast<UParamCometDataAsset>(m_mission.Param))
    {
        // find start position
        TArray<AActor*> starts{};
        UGameplayStatics::GetAllActorsWithTag(_world, Tags::StartComet, starts);

        // spawn actor
        int32 index = FMath::RandRange(0, starts.Num() - 1);
        if (starts[index] == nullptr) return;

        FTransform transform = starts[index]->GetActorTransform();
        FVector const& baseLocation = transform.GetLocation();

        TArray<FVector> delta
        {
            FVector { 760.0f, 760.0f, 760.0f },
            FVector { -340.0f, 180.0f, 340.0f },
            FVector { 0.0, -340.0f, -1000.0f },
            FVector { -500.0f, -700.0f, -230.0f }
        };

        for (int i = 0; i < 5; ++i)
        {
            if (AComet* comet = _world->SpawnActorDeferred<AComet>(param->CometClass, transform))
            {
                m_nbComet++;
                comet->InitialLifeSpan = m_mission.DurationValue;
                comet->OnIntercepDelegate.add(std::bind(&MissionComet::onCometDestroy, this, std::placeholders::_1));

                comet->FinishSpawning(transform);
            }

            if (delta.Num() > 0)
            {
                int32 randDeltaIndex = FMath::RandRange(0, delta.Num() - 1);
                transform.SetTranslation(baseLocation + delta[randDeltaIndex]);
                delta.RemoveAt(randDeltaIndex);
            }
        }
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
                            shipPawn->boostPassive(m_mission.Type, m_mission.RewardValue);
                        }
                    }
                }
            }
        }
        m_teams.Empty();
    }

    if (m_nbComet == 0)
    {
        end(_world);
    }
}

void MissionComet::onCometDestroy(FString const& _team)
{
    m_nbComet--;
    m_teams.Add(_team);
}

void MissionPirate::start(class UWorld* _world)
{
    MissionBehaviour::start(_world);
    if (UParamPirateDataAsset* param = Cast<UParamPirateDataAsset>(m_mission.Param))
    {
        TArray<AActor*> out;
        UGameplayStatics::GetAllActorsWithTag(_world, Tags::StartPirate, out);

        if (out.Num() != 0 && out[0] != nullptr)
        {
            FTransform transform = out[0]->GetTransform();
            if (AActor* actor = UGameplayStatics::BeginDeferredActorSpawnFromClass(_world, param->PirateClass, transform))
            {
                actor->InitialLifeSpan = m_mission.DurationValue;
                if (APirate* pirate = Cast<APirate>(actor))
                {
                    pirate->OnKilledDelegate.add(std::bind(&MissionPirate::onKill, this, std::placeholders::_1));
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
        end(_world);
    }
}

void MissionPirate::onKill(FName const& _team)
{
    m_team = _team;
}

void MissionTakeGold::start(class UWorld* _world)
{
    MissionBehaviour::start(_world);
    if (UParamGoldDataAsset* param = Cast<UParamGoldDataAsset>(m_mission.Param))
    {
        TArray<AActor*> out;
        UGameplayStatics::GetAllActorsWithTag(_world, Tags::StartGold, out);

        // random start
        int32 index = FMath::RandRange(0, out.Num() - 1);

        if (out.Num() != 0 && out[index] != nullptr)
        {
            FTransform transform = out[index]->GetTransform();
            if (AActor* actor = UGameplayStatics::BeginDeferredActorSpawnFromClass(_world, param->GoldClass, transform))
            {
                actor->InitialLifeSpan = m_mission.DurationValue;
                if (AGold* gold = Cast<AGold>(actor))
                {
                    gold->OnDestroyDelegate.add(std::bind(&MissionTakeGold::onTaken, this, std::placeholders::_1));
                }
                UGameplayStatics::FinishSpawningActor(actor, transform);
            }
        }
    }
}

void MissionTakeGold::tick(float _deltaTime, UWorld* _world)
{
    MissionBehaviour::tick(_deltaTime, _world);

    if (m_id != uint32{})
    {
        if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(_world->GetGameState()))
        {
            spacelGameState->OnAskMissionDelegate.Broadcast(EMission::HoldGold);
        }
        end(_world);
    }
}

void MissionTakeGold::onTaken(uint32 const& _id)
{
    m_id = _id;
}

void MissionHoldGold::start(class UWorld* _world)
{
    MissionBehaviour::start(_world);

    findGold(_world);
}

void MissionHoldGold::tick(float _deltaTime, UWorld* _world)
{
    MissionBehaviour::tick(_deltaTime, _world);

    if (m_find)
    {
        findGold(_world);
    }
}

void MissionHoldGold::findGold(class UWorld* _world)
{
    if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(_world->GetGameState()))
    {
        TArray<APlayerState*> playerStates = spacelGameState->PlayerArray;
        for (auto* playerState : playerStates)
        {
            if (AShipPawn* shipPawn = playerState->GetPawn<AShipPawn>())
            {
                if (shipPawn->hasEffect(EEffect::Gold))
                {
                    shipPawn->OnLostGoldDelegate.add(std::bind(&MissionHoldGold::onTokenChange, this));
                    m_find = false;
                    break;
                }
            }
        }
    }
}

void MissionHoldGold::end(UWorld* _world)
{
    MissionBehaviour::end(_world);

    // clean event
    if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(_world->GetGameState()))
    {
        TArray<APlayerState*> playerStates = spacelGameState->PlayerArray;
        for (auto* playerState : playerStates)
        {
            if (AShipPawn* shipPawn = playerState->GetPawn<AShipPawn>())
            {
                shipPawn->OnLostGoldDelegate.clean();
            }
        }
    }
}

void MissionHoldGold::onTokenChange()
{
    if (!isEnd())
    {
        resetTimer();
        m_find = true;
    }
}