// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameState/SpacelGameState.h"
#include "Player/SpacelPlayerState.h"
#include "Player/ShipPawn.h"
#include "Gameplay/Mission/MissionBehaviour.h"

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
			spacelGameState->OnStartGameDelegate.AddDynamic(this, &AMissionManager::OnStartGame);
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
		for (auto& mission : m_openMission)
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

		m_openMission.RemoveAll([](auto& _mission) { return _mission->m_isEnd; });
	
		for (auto& mission : m_silenceMission)
		{
			if (mission.IsValid())
			{
				mission->tick(DeltaTime, this->GetWorld());
				if (mission->m_mustStart)
				{
				}
				else if (mission->m_isEnd)
				{
					endMissionOnNetMulticast(mission->m_mission);
				}
			}
		}

		m_silenceMission.RemoveAll([](auto& _mission) { return _mission->m_isEnd; });
	}
}

void AMissionManager::OnStartGame()
{
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
