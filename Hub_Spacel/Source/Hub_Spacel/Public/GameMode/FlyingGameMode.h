// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameLiftServerSDK.h"
#include "GameFramework/GameModeBase.h"
#include "Http.h"
#include "FlyingGameMode.generated.h"

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

	FUpdateGameSessionState()
	{

	}
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

protected:
	virtual void BeginPlay() override;
	virtual FString InitNewPlayer(class APlayerController* _newPlayerController, FUniqueNetIdRepl const& _uniqueId, FString const& _options, FString const& _portal) override;

private:
	UFUNCTION()
	void CountDownUntilGameOver();

	UFUNCTION()
	void EndGame();

	UFUNCTION()
	void PickAWinningTeam();

	UFUNCTION()
	void HandleProcessTermination();

	UFUNCTION()
	void HandleGameSessionUpdate();

	void onRecordMatchResultResponseReceive(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful);

public:
	UPROPERTY()
	FTimerHandle CountDownUntilGameOverHandle {};

	UPROPERTY()
	FTimerHandle EndGameHandle {};

	UPROPERTY()
	FTimerHandle PickAWinningTeamHandle {};

	UPROPERTY()
	FTimerHandle HandleProcessTerminationHandle {};

	UPROPERTY()
	FTimerHandle HandleGameSessionUpdateHandle {};

	UPROPERTY()
	int RemainingGameTime { 600 };

private:
	class FHttpModule* HttpModule { nullptr };

	UPROPERTY()
	FStartGameSessionState StartGameSessionState;

	UPROPERTY()
	FUpdateGameSessionState UpdateGameSessionState;

	UPROPERTY()
	FProcessTerminateState ProcessTerminateState;

	UPROPERTY()
	FHealthCheckState HealthCheckState;

	UPROPERTY()
	FString ApiUrl {};

	UPROPERTY()
	FString ServerPassword {};

	UPROPERTY()
	bool GameSessionActivated {};
};
