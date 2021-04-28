// Fill out your copyright notice in the Description page of Project Settings.


#include "MissionManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameState/SpacelGameState.h"
#include "DataAsset/MissionDataAsset.h"

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
	}
}

// Called every frame
void AMissionManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMissionManager::OnStartGame()
{
	FTimerDelegate timerCallbackFirstBlood;
	timerCallbackFirstBlood.BindLambda([&]() {
		startMissionOnAllClient(this->MissionDataAsset->get(EMission::FirstBlood)); });

	FTimerHandle handleFirstBlood;
	this->GetWorldTimerManager().SetTimer(handleFirstBlood, timerCallbackFirstBlood, this->MissionDataAsset->get(EMission::FirstBlood).Condition, false);

	FTimerDelegate timerCallbackScoreRace;
	timerCallbackScoreRace.BindLambda([&]() {
		startMissionOnAllClient(this->MissionDataAsset->get(EMission::ScoreRace)); });

	FTimerHandle handleScoreRace;
	this->GetWorldTimerManager().SetTimer(handleScoreRace, timerCallbackScoreRace, this->MissionDataAsset->get(EMission::ScoreRace).Condition, false);
}

void AMissionManager::startMissionOnAllClient(FMission const& _mision) const
{
	UWorld const* world{ this->GetWorld() };
	if (!ensure(world != nullptr)) return;

	if (AGameState* gameState = world->GetGameState())
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
