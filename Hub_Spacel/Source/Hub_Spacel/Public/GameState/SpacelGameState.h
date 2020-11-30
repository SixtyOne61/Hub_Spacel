// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Enum/SpacelEnum.h"
#include "SpacelGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStartPrepare);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStartGame);

USTRUCT()
struct HUB_SPACEL_API FTeamLocation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FString Name {};

	UPROPERTY(EditAnywhere)
	TArray<FTransform> Transforms {};
};

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

	UFUNCTION()
	void AttributePlayersLocation();

protected:
	virtual void BeginPlay() override;

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

	UPROPERTY(EditAnywhere)
	TArray<FTeamLocation> TeamsLocation {};

private:
	UPROPERTY(ReplicatedUsing = OnRep_StateGame)
	uint8 RU_GameState { (uint8)EGameState::Undefined } ;

	TMap<FString, int32> m_scores{ };
};
