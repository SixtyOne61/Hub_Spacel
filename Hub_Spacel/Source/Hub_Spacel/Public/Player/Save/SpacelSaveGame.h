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
	UPROPERTY(VisibleAnywhere, Category = Basic)
	bool HasSeeDitactitial { false };
};
