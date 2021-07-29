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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStartMissionTwoParam, EMission, _type, FName const&, _team);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndMission, EMission, _type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAskMission, EMission, _type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResetTimerMission, EMission, _type);

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
	void GoToLockLowModule() { this->RU_GameState = (uint8)EGameState::LockLowModule; OnRep_StateGame(); }

	UFUNCTION()
	void GoToLockMediumModule() { this->RU_GameState = (uint8)EGameState::LockMediumModule; OnRep_StateGame(); }

	UFUNCTION()
	void GoToLockPrepare() { this->RU_GameState = (uint8)EGameState::LockPrepare; OnRep_StateGame(); }

	UFUNCTION()
	void GoToInGame() { this->RU_GameState = (uint8)EGameState::InGame; OnRep_StateGame(); registerMission(); }

	UFUNCTION()
	void GoToUnlockMedium() { this->RU_GameState = (uint8)EGameState::UnlockMedium; OnRep_StateGame(); }

	UFUNCTION()
	void GoToUnlockUltimate() { this->RU_GameState = (uint8)EGameState::UnlockUltimate; OnRep_StateGame(); }

	UFUNCTION()
	void GoToEndGame() { this->RU_GameState = (uint8)EGameState::EndGame; OnRep_StateGame(); }

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
	void RPCNetMulticastStartMissionTwoParam(EMission _type, FName _team);

	UFUNCTION(Reliable, NetMulticast)
	void RPCNetMulticastEndMission(EMission _type);

	UFUNCTION(Reliable, NetMulticast)
	void RPCNetMulticastResetTimerMission(EMission _type);

protected:
	virtual void BeginPlay() override;

	void registerMission();

	UFUNCTION()
	void CallMission();

private:
	UFUNCTION()
	void OnRep_StateGame();

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
