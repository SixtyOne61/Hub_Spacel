// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Enum/SpacelEnum.h"
#include "SpacelPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUpdateRemainingSkillPoint);

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API ASpacelPlayerState : public APlayerState
{
	GENERATED_BODY()

    friend class USelectorSkillWidget;
    friend class UPreparePhaseWidget;

public:
    inline uint8 getRemainingSkillPoint() const { return RemainingSkillPoint; }

private:
    void setRemainingSkillPoint(uint8 && _val);
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "EShipModuleType"))
    uint8 ShipBaseModuleType = 0x01;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "EShipModuleType"))
    uint8 ShipEngineModuleType = 0x02;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = "EShipModuleType"))
    uint8 ShipShellModuleType = 0x08;

    /* event */

    UPROPERTY(BlueprintAssignable)
    FUpdateRemainingSkillPoint OnUpdateRemainingSkillPointDelegate {};

    /* Network */

    UPROPERTY()
    FString PlayerSessionId {};

    UPROPERTY()
    FString MatchmakingPlayerId {};

    UPROPERTY(Replicated)
    FString Team {};

private:
    UPROPERTY()
    uint8 RemainingSkillPoint { 4 };

    TMap<ESkillType, uint8> m_skillPoints { 
        { ESkillType::Attack, 0 },
        { ESkillType::Protection, 0 }, 
        { ESkillType::Support, 0 } };
};
