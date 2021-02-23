// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Enum/SpacelEnum.h"
#include "SpacelGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStartPrepare);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStartGame);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FScoreUpdate);

USTRUCT()
struct HUB_SPACEL_API FTeamLocation
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FString Name {};

	UPROPERTY(EditAnywhere)
	TArray<FTransform> Transforms {};
};

USTRUCT()
struct HUB_SPACEL_API FScore
{
	GENERATED_BODY()

	FScore() = default;

	FScore(FString const& _team, int32 _score)
		: Team(_team)
		, Score(_score)
		{};

	UPROPERTY()
	FString Team {};

	UPROPERTY()
	int32 Score { 0 };
};

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API ASpacelGameState : public AGameStateBase
{
	GENERATED_BODY()

	friend class USpacelWidget;

public:
	UFUNCTION()
	void GoToPrepare() { this->RU_GameState = (uint8)EGameState::Prepare; OnRep_StateGame(); }

	UFUNCTION()
	void GoToInGame() { this->RU_GameState = (uint8)EGameState::InGame; OnRep_StateGame(); }

	FString GetBestTeam() const;

	void AddScore(FString const& _team, EScoreType _type);

	UFUNCTION()
	void RegisterTeam();

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

	UPROPERTY()
	FScoreUpdate OnScoreUpdateDelegate {};

private:
	UPROPERTY(ReplicatedUsing = OnRep_StateGame)
	uint8 RU_GameState { (uint8)EGameState::Undefined } ;

	UPROPERTY(Replicated)
	TArray<FScore> R_Scores {};
};
