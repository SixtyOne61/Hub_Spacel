// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Enum/SpacelEnum.h"
#include <functional>
#include "SpacelPlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSkillLobbyChange);

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API ASpacelPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
    void BeginPlay() override;

    UFUNCTION(Reliable, Server)
    void RPCAddSkill(uint8 const& _id, ESkillType _type);

    void LocalAddSkill(uint8 const& _id, ESkillType _type);

    UFUNCTION()
    void SetTeam(FString const& _team);

    uint8 getSkillId(ESkillType _type) const;

private:
    /* for call set collision profile on pawn */
    UFUNCTION()
    void WaitPawnCreation();
	
public:
    /* Network */

    UPROPERTY()
    FString PlayerSessionId {};

    UPROPERTY()
    FString MatchmakingPlayerId {};

    UPROPERTY(Replicated, BlueprintReadWrite)
    FString R_Team { "Team 1" };

    UPROPERTY(Replicated)
    uint8 R_LowSkill {};

    UPROPERTY(Replicated)
    uint8 R_MediumSkill {};

    UPROPERTY(Replicated)
    uint8 R_HightSkill {};

    UPROPERTY()
    FOnSkillLobbyChange OnSkillLobbyChangeDelegate {};

    FTransform PlayerStartTransform {};

    std::function<void(class SkillCountDown*)> OnAddSkillUniqueDelegate { nullptr };
};
