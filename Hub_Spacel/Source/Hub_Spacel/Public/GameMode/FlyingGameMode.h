// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameLiftServerSDK.h"
#include "GameFramework/GameModeBase.h"
#include "Http.h"
#include "FlyingGameMode.generated.h"

UENUM()
enum class EUpdateReason : uint8
{
	NO_UPDATE_RECEIVED,
	BACKFILL_INITIATED,
	MATCHMAKING_DATA_UPDATED,
	BACKFILL_FAILED,
	BACKFILL_TIMED_OUT,
	BACKFILL_CANCELLED,
	BACKFILL_COMPLETED
};

USTRUCT()
struct FStartGameSessionState
{
	GENERATED_BODY();

	UPROPERTY()
	bool Status { false };

	UPROPERTY()
	FString MatchmakingConfigurationArn {};

	TMap<FString, Aws::GameLift::Server::Model::Player> PlayerIdToPlayer {};
};

USTRUCT()
struct FUpdateGameSessionState
{
	GENERATED_BODY();

	UPROPERTY()
	EUpdateReason Reason { EUpdateReason::NO_UPDATE_RECEIVED };

	TMap<FString, Aws::GameLift::Server::Model::Player> PlayerIdToPlayer {};

	FUpdateGameSessionState() = default;
};

USTRUCT()
struct FProcessTerminateState
{
	GENERATED_BODY();

	FProcessTerminateState()
	{

	}

	UPROPERTY()
	bool Status { false };

	long m_terminationTime { };
};

USTRUCT()
struct FHealthCheckState
{
	GENERATED_BODY();

	UPROPERTY()
	bool Status { false };
};
/**
 * 
 */
UCLASS()
class HUB_SPACEL_API AFlyingGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	AFlyingGameMode();

	virtual void PreLogin(FString const& _options, FString const& _address, FUniqueNetIdRepl const& _uniqueId, FString & _errorMessage) override;
	virtual void Logout(class AController* _exiting) override;

	UFUNCTION(BlueprintCallable)
	FTransform GetSpawnLocation(FName const& _team);

protected:
	void BeginPlay() override;
	FString InitNewPlayer(class APlayerController* _newPlayerController, FUniqueNetIdRepl const& _uniqueId, FString const& _options, FString const& _portal) override;
	void Tick(float _deltaSeconde) override;

private:
	UFUNCTION()
	void EndLobby();

	UFUNCTION()
	void EndGame();

	UFUNCTION()
	void PickAWinningTeam();

	UFUNCTION()
	void LeaveLevel();

	UFUNCTION()
	void HandleProcessTermination();

	UFUNCTION()
	void HandleGameSessionUpdate();

	void onRecordMatchResultResponseReceive(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful);

	UFUNCTION()
	void SuspendBackfill();

	FString createBackfillRequest(FString const& _gameSessionArn, FString const& _matchmakingConfigurationArn, TMap<FString, Aws::GameLift::Server::Model::Player> const& _players);

	bool stopBackfillRequest(FString const& _gameSessionArn, FString const& _matchmakingConfigurationArn, FString const& _ticketId);

public:
	UPROPERTY()
	FTimerHandle EndGameHandle {};

	UPROPERTY()
	FTimerHandle LeaveLevelHandle {};

	UPROPERTY()
	FTimerHandle HandleProcessTerminationHandle {};

	UPROPERTY()
	FTimerHandle HandleGameSessionUpdateHandle {};

	UPROPERTY()
	FTimerHandle SuspendBackfillHandle {};

	UPROPERTY()
	FTimerHandle UnlockInputHandle {};

	UPROPERTY()
	int RemainingGameTime { }; // 11'30

	UPROPERTY()
	int RemainingChooseModuleTime { }; // 60

	UPROPERTY()
	int RemainingLeaveTime { };

	UPROPERTY()
	int SuspendBackfillTime { 45 };

	UPROPERTY()
	int MaxPlayerPerGame { 18 };

protected:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UFlyingGameModeDataAsset* GameModeDataAsset{ nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UEditorHackDataAsset* HackDataAsset { nullptr };

private:
	class FHttpModule* HttpModule { nullptr };

	UPROPERTY()
	FStartGameSessionState StartGameSessionState {};

	UPROPERTY()
	FUpdateGameSessionState UpdateGameSessionState {};

	UPROPERTY()
	FProcessTerminateState ProcessTerminateState {};

	UPROPERTY()
	FHealthCheckState HealthCheckState {};

	UPROPERTY()
	FString ApiUrl {};

	UPROPERTY()
	FString ServerPassword {};

	UPROPERTY()
	bool GameSessionActivated { false };

	UPROPERTY()
	FString LatestBackfillTicketId {};

	UPROPERTY()
	bool WaitingForPlayersToJoin { false };

	UPROPERTY()
	int TimeSpentWaitingForPlayersToJoin { 0 };

	TMap<FString, Aws::GameLift::Server::Model::Player> m_expectedPlayers {};

	struct FStartLocation
	{
		bool m_isUse { false };
		FTransform m_transform {};
	};

	TMap<FName, TArray<FStartLocation>> m_startLocation {};

	float m_timerSeconde { 0.0f };
};
