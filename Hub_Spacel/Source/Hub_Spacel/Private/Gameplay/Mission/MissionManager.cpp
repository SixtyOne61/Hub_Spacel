// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameState/SpacelGameState.h"
#include "Player/SpacelPlayerState.h"
#include "Player/ShipPawn.h"
#include "Gameplay/Mission/MissionBehaviour.h"
#include "Gameplay/Mission/Comet.h"
#include "Util/Tag.h"
#include "DataAsset/EditorHackDataAsset.h"
#include <functional>

// Sets default values
AMissionManager::AMissionManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AMissionManager::BeginPlay()
{
	Super::BeginPlay();
	
	if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())))
		{
			spacelGameState->OnAskMissionDelegate.AddDynamic(this, &AMissionManager::OnAskMission);
		}

		OnAskMission(EMission::EcartType);
	}
}

void AMissionManager::batch()
{
	if (this->MissionDataAsset == nullptr) return;

	for (auto missionId : m_batch)
	{
		FMission const& mission = this->MissionDataAsset->getMission(missionId);

		switch (missionId)
		{
		case EMission::Pirate:
		{
			m_openMission.Add(MakeUnique<MissionPirate>(mission));
			startMission(m_openMission.Last());
			break;
		}

		case EMission::Comet:
		{
			m_openMission.Add(MakeUnique<MissionComet>(mission));
			startMission(m_openMission.Last());
			break;
		}

		case EMission::TakeGold:
		{
			m_openMission.Add(MakeUnique<MissionTakeGold>(mission));
			startMission(m_openMission.Last());
			break;
		}

		case EMission::HoldGold:
		{
			m_openMission.Add(MakeUnique<MissionHoldGold>(mission));
			startMission(m_openMission.Last());
			break;
		}

		case EMission::EcartType:
		{
			m_silenceMission.Add(MakeUnique<MissionEcartType>(mission));
			break;
		}

		case EMission::FirstBlood:
		{
			// start mission first blood with delay
			FTimerDelegate timerCallbackFirstBlood;
			timerCallbackFirstBlood.BindLambda([&]() {
				FMission const& firstblood = this->MissionDataAsset->getMission(EMission::FirstBlood);
				m_openMission.Add(MakeUnique<MissionFirstBlood>(firstblood));
				startMission(m_openMission.Last()); });

			FTimerHandle handleFirstBlood;
			this->GetWorldTimerManager().SetTimer(handleFirstBlood, timerCallbackFirstBlood, mission.ConditionValue, false);
			break;
		}

		case EMission::ScoreRace:
		{
			// start mission score race with delay
			FTimerDelegate timerCallbackScoreRace;
			timerCallbackScoreRace.BindLambda([&]() {
				FMission const& scoreRace = this->MissionDataAsset->getMission(EMission::ScoreRace);
				m_openMission.Add(MakeUnique<MissionRaceScore>(scoreRace));
				startMission(m_openMission.Last()); });

			FTimerHandle handleScoreRace;
			this->GetWorldTimerManager().SetTimer(handleScoreRace, timerCallbackScoreRace, mission.ConditionValue, false);
			break;
		}

		default:
			ensure(false);
			break;
		}
	}

	m_batch.Empty();
}

void AMissionManager::OnAskMission(EMission _missionId)
{
	m_batch.Add(_missionId);
}

// Called every frame
void AMissionManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		auto lb = [&](auto& _missions)
		{
			for (auto& mission : _missions)
			{
				if (mission.IsValid())
				{
					mission->tick(DeltaTime, this->GetWorld());
					if (mission->m_isEnd)
					{
						endMission(mission->m_mission);
					}
				}
			}

			_missions.RemoveAll([](auto& _mission) { return _mission->m_isEnd; });
		};

#if WITH_EDITOR
		if (HackDataAsset != nullptr && HackDataAsset->UseHack && HackDataAsset->MissionSucceedImmediately)
		{
			for (auto& mission : m_openMission)
			{
				if (mission.IsValid())
				{
					mission->end();
				}
			}
		}
#endif
		batch();
		lb(m_openMission);
		lb(m_silenceMission);
	}
}

void AMissionManager::startMission(TUniquePtr<MissionBehaviour>& _missionBehaviour)
{
	if(!_missionBehaviour.IsValid()) return;

	UWorld const* world{ this->GetWorld() };
	if (!ensure(world != nullptr)) return;

	if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(world->GetGameState()))
	{
		spacelGameState->RPCNetMulticastStartMission(_missionBehaviour->m_mission.Type);
	}

	_missionBehaviour->OnResetTimerUniqueDelegate.add(std::bind(&AMissionManager::onResetTimer, this, std::placeholders::_1));
}

void AMissionManager::endMission(FMission const& _mission) const
{
	UWorld const* world{ this->GetWorld() };
	if (!ensure(world != nullptr)) return;

	if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(world->GetGameState()))
	{
		spacelGameState->RPCNetMulticastEndMission(_mission.Type);
	}
}

void AMissionManager::onResetTimer(EMission _type)
{
	UWorld const* world{ this->GetWorld() };
	if (!ensure(world != nullptr)) return;

	if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(world->GetGameState()))
	{
		spacelGameState->RPCNetMulticastResetTimerMission(_type);
	}
}
