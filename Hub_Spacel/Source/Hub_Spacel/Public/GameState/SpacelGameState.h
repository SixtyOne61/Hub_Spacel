// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Enum/SpacelEnum.h"
#include "DataAsset/MissionDataAsset.h"
#include "SpacelGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStartPrepare);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLockPrepare);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FStartGame);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FScoreUpdate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayerEnterFog, int32, _playerId, bool, _enter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnlockSkill, EGameState, _state);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStartMission, FMission, _mission);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndMission, FMission, _mission);

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
	friend class UScoreUserWidget;
	friend class AShipPawn;

public:
	UFUNCTION()
	void GoToPrepare() { this->RU_GameState = (uint8)EGameState::Prepare; OnRep_StateGame(); }

	UFUNCTION()
	void GoToLockPrepare() { this->RU_GameState = (uint8)EGameState::LockPrepare; OnRep_StateGame(); }

	UFUNCTION()
	void GoToInGame() { this->RU_GameState = (uint8)EGameState::InGame; OnRep_StateGame(); }

	UFUNCTION()
	void GoToUnlockMedium() { this->RU_GameState = (uint8)EGameState::UnlockMedium; OnRep_StateGame(); }

	UFUNCTION()
	void GoToUnlockUltimate() { this->RU_GameState = (uint8)EGameState::UnlockUltimate; OnRep_StateGame(); }

	UFUNCTION()
	FString GetBestTeam() const;

	UFUNCTION()
	FString GetWorstTeam() const;

	UFUNCTION()
	int32 GetScore(FString const& _team) const;

	void AddScore(FString const& _team, int32 _playerId, EScoreType _type);
	void AddScore(FString const& _team, int32 _playerId, EScoreType _type, int32 _nb);

	UFUNCTION()
	void RegisterTeam();

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnRep_StateGame();

	void teamScoreBoost();

	UFUNCTION(Reliable, NetMulticast)
	void RPCNetMulticastScoreBoost(FMission const& _mission);

public:
	UPROPERTY(Replicated)
	FString R_LatestEvent {};

	UPROPERTY(Replicated)
	FString R_WinningTeam {};

	UPROPERTY()
	FStartPrepare OnStartPrepareDelegate {};

	UPROPERTY()
	FLockPrepare OnLockPrepareDelegate {};

	UPROPERTY()
	FStartGame OnStartGameDelegate {};

	UPROPERTY()
	FScoreUpdate OnScoreUpdateDelegate {};

	UPROPERTY()
	FPlayerEnterFog OnPlayerEnterFogDelegate {};

	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnUnlockSkill OnUnlockSkillDelegate {};

	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnStartMission OnStartMissionDelegate {};

	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnEndMission OnEndMissionDelegate {};

protected:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UGameStateDataAsset* GameStateDataAsset { nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UMissionDataAsset* MissionDataAsset{ nullptr };

private:
	UPROPERTY(ReplicatedUsing = OnRep_StateGame)
	uint8 RU_GameState { (uint8)EGameState::Undefined } ;

	UPROPERTY(Replicated)
	TArray<FScore> R_Scores {};

	TSet<FString> TeamWithBonusMission {};
};
