// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SpacelSaveGame.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API USpacelSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	USpacelSaveGame();

	UPROPERTY(VisibleAnywhere, Category = Basic)
	bool HasSeeDitactitial { false };

	UPROPERTY(VisibleAnywhere, Category = Basic)
	FString PlayerName;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	FString SaveSlotName;

	UPROPERTY(VisibleAnywhere, Category = Basic)
	uint32 UserIndex;
};
