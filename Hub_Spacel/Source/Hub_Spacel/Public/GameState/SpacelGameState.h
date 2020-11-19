// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SpacelGameState.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API ASpacelGameState : public AGameStateBase
{
	GENERATED_BODY()
public:
	UPROPERTY(Replicated)
	FString LatestEvent {};

	UPROPERTY(Replicated)
	FString WinningTeam {};
};
