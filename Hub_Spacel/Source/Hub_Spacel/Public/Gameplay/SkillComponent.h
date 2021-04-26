// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/PlayerActorComponent.h"
#include "Gameplay/Skill/SkillCountDown.h"
#include "SkillComponent.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API USkillComponent : public UPlayerActorComponent
{
	GENERATED_BODY()

	friend class AShipPawn;
	
public:
	USkillComponent();

	inline TArray<TUniquePtr<SkillCountDown>> const& getSkills() const { return m_skills; }

protected:
	void TickComponent(float _deltaTime, ELevelTick _tickType, FActorComponentTickFunction* _thisTickFunction) override;

private:
	void setupSkill();

	UFUNCTION(Reliable, Server)
	void RPCServerUseSkill(ESkill _skill);

	UFUNCTION(Reliable, Client)
	void RPCClientSucced(ESkill _skill);

	UFUNCTION(Reliable, Client)
	void RPCClientFailed(ESkill _skill);

	UFUNCTION()
	void OnUnlockUltimate();

	UFUNCTION()
	void OnLooserTeamUnlockLvl2(FName const& _loosingTeam);

	UFUNCTION()
	void DelayUnlockLvl2();

public:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class USkillDataAsset* SkillDataAsset{ nullptr };

	// temp need setup input
	UPROPERTY(Category = "Setup", EditAnywhere, BlueprintReadWrite)
	TArray<FKey> DefaultKeyboard {};

private:
	TArray<TUniquePtr<SkillCountDown>> m_skills {};
};
