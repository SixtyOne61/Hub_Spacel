// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Enum/SpacelEnum.h"
#include <functional>
#include "SpacelPlayerState.generated.h"

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
    void RPCServerAddSkill(ESkill _id, ESkillType _type);

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
    uint8 R_LowSkill { (uint8)ESkill::DefaultLow };

    UPROPERTY(Replicated)
    uint8 R_MediumSkill { (uint8)ESkill::DefaultMedium };

    UPROPERTY(Replicated)
    uint8 R_HightSkill { (uint8)ESkill::DefaultHight };

    UPROPERTY(Replicated)
    int32 R_KilledByPlayerId {};

    std::function<void(class SkillCountDown*)> OnAddSkillUniqueDelegate { nullptr };
    std::function<void(ESkill _type)> OnRemoveSkillUniqueDelegate { nullptr };
};
