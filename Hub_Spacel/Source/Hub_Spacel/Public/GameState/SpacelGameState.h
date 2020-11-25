// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Enum/SpacelEnum.h"
#include "SpacelGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStartPrepare);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStartGame);

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API ASpacelGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void GoToPrepare() { this->RU_GameState = (uint8)EGameState::Prepare; }

	UFUNCTION()
	void GoToInGame() { this->RU_GameState = (uint8)EGameState::InGame; }

	FString GetBestTeam() const;

	void AddScore(FString const& _team, int32 _val);

private:
	UFUNCTION()
	void OnRep_StateGame();

public:
	UPROPERTY(Replicated)
	FString R_LatestEvent {};

	UPROPERTY(Replicated)
	FString R_WinningTeam {};

	UPROPERTY()
	FStartPrepare OnStartPrepareDelegate {};

	UPROPERTY()
	FStartGame OnStartGameDelegate {};

private:
	UPROPERTY(ReplicatedUsing = OnRep_StateGame)
	uint8 RU_GameState { (uint8)EGameState::Undefined } ;

	TMap<FString, int32> m_scores{ { "Team 1", 0 } };
};
