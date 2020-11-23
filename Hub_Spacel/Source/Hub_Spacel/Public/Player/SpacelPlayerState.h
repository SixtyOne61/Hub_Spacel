// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SpacelPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API ASpacelPlayerState : public APlayerState
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "EShipModuleType"))
    uint8 ShipBaseModuleType = 0x01;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "EShipModuleType"))
    uint8 ShipEngineModuleType = 0x02;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "EShipModuleType"))
    uint8 ShipShellModuleType = 0x08;

    /* Network */

    UPROPERTY()
    FString PlayerSessionId {};

    UPROPERTY()
    FString MatchmakingPlayerId {};

    UPROPERTY(Replicated)
    FString Team {};
};
