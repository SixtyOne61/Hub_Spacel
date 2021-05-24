// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameState/SpacelGameState.h"
#include "Player/SpacelPlayerState.h"
#include "Player/ShipPawn.h"
#include "Gameplay/Mission/MissionBehaviour.h"
#include "Gameplay/Mission/Comet.h"
#include "Util/Tag.h"
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
			spacelGameState->OnChangeStateDelegate.AddDynamic(this, &AMissionManager::OnStartGame);
		}

		FMission const& mission = this->MissionDataAsset->getMission(EMission::EcartType);
		m_silenceMission.Add(MakeUnique<MissionEcartType>(mission));
	}
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
						endMissionOnNetMulticast(mission->m_mission);
					}
				}
			}

			_missions.RemoveAll([](auto& _mission) { return _mission->m_isEnd; });
		};
		
		lb(m_openMission);
		lb(m_silenceMission);
	}
}

void AMissionManager::OnStartGame(EGameState _state)
{
	if(_state != EGameState::InGame) return;
	if(this->MissionDataAsset == nullptr) return;

	FMission const& firstBlood = this->MissionDataAsset->getMission(EMission::FirstBlood);
	FMission const& scoreRace = this->MissionDataAsset->getMission(EMission::ScoreRace);

	// start mission first blood with delay
	FTimerDelegate timerCallbackFirstBlood;
	timerCallbackFirstBlood.BindLambda([&]() {
		FMission const& mission = this->MissionDataAsset->getMission(EMission::FirstBlood);
		m_openMission.Add(MakeUnique<MissionFirstBlood>(mission));
		startMissionOnAllClient(m_openMission.Last()->m_mission); });

	FTimerHandle handleFirstBlood;
	this->GetWorldTimerManager().SetTimer(handleFirstBlood, timerCallbackFirstBlood, firstBlood.ConditionValue, false);

	// start mission score race with delay
	FTimerDelegate timerCallbackScoreRace;
	timerCallbackScoreRace.BindLambda([&]() {
		FMission const& mission = this->MissionDataAsset->getMission(EMission::ScoreRace);
		m_openMission.Add(MakeUnique<MissionFirstBlood>(mission));
		startMissionOnAllClient(this->MissionDataAsset->getMission(EMission::ScoreRace)); });
	
	FTimerHandle handleScoreRace;
	this->GetWorldTimerManager().SetTimer(handleScoreRace, timerCallbackScoreRace, scoreRace.ConditionValue, false);
}

void AMissionManager::startMissionOnAllClient(FMission const& _mission) const
{
	UWorld const* world{ this->GetWorld() };
	if (!ensure(world != nullptr)) return;

	if (ASpacelGameState* gameState = world->GetGameState<ASpacelGameState>())
	{
		TArray<APlayerState*> playerStates = gameState->PlayerArray;
		for (auto* playerState : playerStates)
		{
			if (AShipPawn* shipPawn = playerState->GetPawn<AShipPawn>())
			{
				shipPawn->RPCClientStartMission(_mission);
			}
		}
	}
}

void AMissionManager::endMissionOnNetMulticast(FMission const& _mission) const
{
	UWorld const* world{ this->GetWorld() };
	if (!ensure(world != nullptr)) return;

	if (ASpacelGameState* gameState = world->GetGameState<ASpacelGameState>())
	{
		TArray<APlayerState*> playerStates = gameState->PlayerArray;
		for (auto* playerState : playerStates)
		{
			if (AShipPawn* shipPawn = playerState->GetPawn<AShipPawn>())
			{
				shipPawn->RPCNetMulticastEndMission(_mission);
			}
		}
	}
}

void AMissionManager::startMissionComet()
{
	if (this->CometClass == nullptr) return;

	UWorld* const world{ this->GetWorld() };
	if (!ensure(world != nullptr)) return;

	// find start position
	TArray<AActor*> starts {};
	UGameplayStatics::GetAllActorsWithTag(world, Tags::PointStartComet, starts);

	// spawn actor
	int32 index = FMath::RandRange(0, starts.Num()-1);
	if(starts[index] == nullptr) return;

	FTransform transform = starts[index]->GetActorTransform();
	FVector const& baseLocation = transform.GetLocation();

	TArray<FVector> scales {
		FVector { 20.0f, 3.0f, 5.0f },
		FVector { 7.0f, 5.0f, 1.0f },
		FVector { 1.0f, 1.0f, 1.0f },
		FVector { 11.0f, 4.0f, 4.0f }
	};

	TArray<FVector> delta
	{
		FVector { 860.0f, 860.0f, 860.0f },
		FVector { -640.0f, 180.0f, 440.0f },
		FVector { 0.0, -440.0f, -1200.0f },
		FVector { -600.0f, -800.0f, -330.0f }
	};

	transform.SetScale3D(FVector{ 30.0f, 7.0f, 7.0f });

	for (int i = 0; i < 5; ++i)
	{
		AComet* comet = world->SpawnActorDeferred<AComet>(this->CometClass, transform);
		if (comet)
		{
			FMission const& cometMission = this->MissionDataAsset->getMission(EMission::Comet);
			m_openMission.Add(MakeUnique<MissionComet>(cometMission));
			startMissionOnAllClient(this->MissionDataAsset->getMission(EMission::Comet));

			MissionComet* missionPtr = static_cast<MissionComet*>(m_openMission.Last().Get());
			missionPtr->m_nbComet++;
			comet->m_onIntercep.add(std::bind(&MissionComet::onCometDestroy, missionPtr, std::placeholders::_1));

			comet->FinishSpawning(transform);
		}

		if (scales.Num() > 0 && delta.Num() > 0)
		{
			int32 randScaleIndex = FMath::RandRange(0, scales.Num() - 1);
			transform.SetScale3D(scales[randScaleIndex]);

			int32 randDeltaIndex = FMath::RandRange(0, delta.Num() - 1);
			transform.SetTranslation(baseLocation + delta[randDeltaIndex]);
			delta.RemoveAt(randDeltaIndex);
		}
	}
}
