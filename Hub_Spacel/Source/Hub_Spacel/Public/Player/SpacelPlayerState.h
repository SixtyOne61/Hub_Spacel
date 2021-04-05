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
    void BeginPlay() override;

    inline uint8 getRemainingSkillPoint() const { return RemainingSkillPoint; }

    uint8 getSkillPoint(ESkillType const& _type) const;

    UFUNCTION(Reliable, Server)
    void RPCSetSkillPoint(ESkillType const& _type, uint8 _value);

    bool IsIncreaseFireRate() const { return Attack > 0; }

    UFUNCTION()
    void SetTeam(FString const& _team);

private:
    void setRemainingSkillPoint(uint8 && _val);

    /* for call set collision profile on pawn */
    UFUNCTION()
    void WaitPawnCreation();
	
public:
    static constexpr uint8 MaxSkillPoint { 4 };
    static constexpr uint8 MaxSkillPointType { 3 };

    /* event */

    UPROPERTY(BlueprintAssignable)
    FUpdateRemainingSkillPoint OnUpdateRemainingSkillPointDelegate {};

    /* Network */

    UPROPERTY()
    FString PlayerSessionId {};

    UPROPERTY()
    FString MatchmakingPlayerId {};

    UPROPERTY(Replicated, BlueprintReadWrite)
    FString Team { "Team 1" };

    UPROPERTY(Replicated)
    uint8 Attack {};

    UPROPERTY(Replicated)
    uint8 Protection {};

    UPROPERTY(Replicated)
    uint8 Support {};

    FTransform PlayerStartTransform {};

private:
    UPROPERTY()
    uint8 RemainingSkillPoint { MaxSkillPoint };
};
