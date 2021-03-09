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

protected:
	void TickComponent(float _deltaTime, ELevelTick _tickType, FActorComponentTickFunction* _thisTickFunction) override;

private:
	void setupSkill();
	void useSkill(float _slot);

public:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class USkillDataAsset* SkillDataAsset{ nullptr };

private:
	TArray<TUniquePtr<SkillCountDown>> m_skills {};
};
