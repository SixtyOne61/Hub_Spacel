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

    inline uint8 getSkillPoint(ESkillType _type) const { return m_skillPoints[_type]; }

private:
    void setRemainingSkillPoint(uint8 && _val);
	
public:
    /* event */

    UPROPERTY(BlueprintAssignable)
    FUpdateRemainingSkillPoint OnUpdateRemainingSkillPointDelegate {};

    /* Network */

    UPROPERTY()
    FString PlayerSessionId {};

    UPROPERTY()
    FString MatchmakingPlayerId {};

    UPROPERTY(Replicated)
    FString Team { "Team 1" };

private:
    UPROPERTY()
    uint8 RemainingSkillPoint { 4 };

    TMap<ESkillType, uint8> m_skillPoints { 
        { ESkillType::Attack, 0 },
        { ESkillType::Protection, 0 }, 
        { ESkillType::Support, 0 } };
};
