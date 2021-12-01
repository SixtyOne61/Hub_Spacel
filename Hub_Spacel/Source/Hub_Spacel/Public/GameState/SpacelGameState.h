// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Enum/SpacelEnum.h"
#include "SpacelGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChangeState, EGameState, _state);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FScoreUpdate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayerEnterFog, int32, _playerId, bool, _enter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStartMission, EMission, _type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnStartMissionTwoParam, EMission, _type, FName const&, _team, FName const&, _teamTarget);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEndMission, EMission, _type, bool, _succeed, FName, _succeedForTeam);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAskMission, EMission, _type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResetTimerMission, EMission, _type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWhoKillWho, int32, _killer, int32, _killed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStartLocalTimer, int, _startTime);

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

USTRUCT(BlueprintType)
struct HUB_SPACEL_API FPlayerData
{
	GENERATED_BODY()

	FPlayerData() = default;

	FPlayerData(int32 _playerId, FName const& _team, FName const& _playerName, ESkill _lowSkill, ESkill _mediumSkill, ESkill _hightSkill)
		: PlayerId(_playerId)
		, Team(_team)
		, PlayerName(_playerName)
		, LowSkill(_lowSkill)
		, MediumSkill(_mediumSkill)
		, HightSkill(_hightSkill)
	{};

	UPROPERTY()
	int32 PlayerId {};

	UPROPERTY(BlueprintReadOnly)
	FName Team { "" };

	UPROPERTY(BlueprintReadOnly)
	FName PlayerName { "" };

	UPROPERTY(BlueprintReadOnly)
	ESkill LowSkill { ESkill::DefaultLow };

	UPROPERTY(BlueprintReadOnly)
	ESkill MediumSkill { ESkill::DefaultMedium };

	UPROPERTY(BlueprintReadOnly)
	ESkill HightSkill { ESkill::DefaultHight };
};

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API ASpacelGameState : public AGameStateBase
{
	GENERATED_BODY()

	friend class USpacelWidget;
	friend class UInGameWidget;
	friend class AShipPawn;

public:
	UFUNCTION()
	void GoToPrepare() { this->RU_GameState = (uint8)EGameState::Prepare; OnRep_StateGame(); }

	UFUNCTION()
	void GoToChooseSKill() { this->RU_GameState = (uint8)EGameState::ChooseSkill; OnRep_StateGame(); }

	UFUNCTION()
	void GoToLockPrepare() { this->RU_GameState = (uint8)EGameState::LockPrepare; OnRep_StateGame(); }

	UFUNCTION()
	void GoToInGame() { this->RU_GameState = (uint8)EGameState::InGame; OnRep_StateGame(); registerMission(); }

	UFUNCTION()
	void GoToEndGame() { this->RU_GameState = (uint8)EGameState::EndGame; OnRep_StateGame(); }

	UFUNCTION()
	void GoToWaitEnd() { this->RU_GameState = (uint8)EGameState::WaitEnd; OnRep_StateGame(); }

	UFUNCTION()
	FString GetBestTeam() const;

	UFUNCTION()
	FString GetWorstTeam() const;

	UFUNCTION()
	int32 GetScore(FString const& _team) const;

	void AddScore(FString const& _team, int32 _playerId, EScoreType _type);
	void AddScore(FString const& _team, int32 _playerId, EScoreType _type, int32 _nb);
	void AddScore(FString const& _team, int32 _value);

	UFUNCTION()
	void RegisterTeam();

	UFUNCTION()
	inline EGameState GetState() const { return (EGameState)this->RU_GameState; }

	UFUNCTION(Reliable, NetMulticast)
	void RPCNetMulticastStartMission(EMission _type);

	UFUNCTION(Reliable, NetMulticast)
	void RPCNetMulticastStartMissionTwoParam(EMission _type, FName _team, FName _teamTarget);

	UFUNCTION(Reliable, NetMulticast)
	void RPCNetMulticastEndMission(EMission _type, bool _succeed, FName _succeedForTeam);

	UFUNCTION(Reliable, NetMulticast)
	void RPCNetMulticastResetTimerMission(EMission _type);

	UFUNCTION(Reliable, NetMulticast)
	void RPCNetMulticastKill(int32 _killer, int32 _killed);

	UFUNCTION(Reliable, NetMulticast)
	void RPCNetMulticastStartGlobalCountDown(int64 _syncPoint, uint16 _duration);

protected:
	void BeginPlay() override;

	void registerMission();

	UFUNCTION()
	void CallMission();

private:
	UFUNCTION()
	void OnRep_StateGame();

	/* call on client, leave to end menu */
	void travelToEndMenu();

	/* call on server, to register player */
	void registerPlayerData();

	/* call on client, to save metric to game instance */
	void sendMetric();

public:
	UPROPERTY(Replicated)
	FString R_LatestEvent {};

	UPROPERTY(Replicated)
	FString R_WinningTeam {};

	UPROPERTY(BlueprintAssignable, EditAnywhere, BlueprintReadWrite)
	FChangeState OnChangeStateDelegate {};

	UPROPERTY()
	FScoreUpdate OnScoreUpdateDelegate {};

	UPROPERTY()
	FPlayerEnterFog OnPlayerEnterFogDelegate {};

	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnStartMission OnStartMissionDelegate {};

	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnStartMissionTwoParam OnStartMissionTwoParamDelegate {};

	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnEndMission OnEndMissionDelegate {};

	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnResetTimerMission OnResetTimerMissionDelegate {};

	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnAskMission OnAskMissionDelegate {};

	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnWhoKillWho OnWhoKillWhoDelegate {};

	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FStartLocalTimer OnStartLocalTimerDelegate {};

	UPROPERTY()
	int GlobalSecondLocalCountDown { 0 };

	UPROPERTY(Replicated)
	TArray<FPlayerData> R_PlayersData {};

protected:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UGameStateDataAsset* GameStateDataAsset { nullptr };

	TArray<EMission> RandomMissions { EMission::Comet, EMission::Pirate };

private:
	UPROPERTY(ReplicatedUsing = OnRep_StateGame)
	uint8 RU_GameState { (uint8)EGameState::Undefined } ;

	UPROPERTY(Replicated)
	TArray<FScore> R_Scores {};

	TSet<FString> TeamWithBonusMission {};
};
