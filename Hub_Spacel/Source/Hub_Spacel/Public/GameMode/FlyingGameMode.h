// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameLiftServerSDK.h"
#include "GameFramework/GameModeBase.h"
#include "FlyingGameMode.generated.h"

USTRUCT()
struct FStartGameSessionState
{
	GENERATED_BODY();

	UPROPERTY()
	bool Status { false };
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
	
protected:
    AFlyingGameMode();

	virtual void BeginPlay() override;

private:
	UPROPERTY()
	FStartGameSessionState StartGameSessionState;

	UPROPERTY()
	FUpdateGameSessionState UpdateGameSessionState;

	UPROPERTY()
	FProcessTerminateState ProcessTerminateState;

	UPROPERTY()
	FHealthCheckState HealthCheckState;
};
