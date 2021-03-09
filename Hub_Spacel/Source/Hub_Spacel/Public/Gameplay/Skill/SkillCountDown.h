// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAsset/SkillDataAsset.h"
#include "Enum/SpacelEnum.h"
#include "Util/EnumUtil.h"

/**
 * 
 */
class HUB_SPACEL_API SkillCountDown
{
public:
	SkillCountDown(const SkillCountDown &);
	SkillCountDown(FSkill _skill, class AShipPawn* _pawn);
	~SkillCountDown();

	void use(class UWorld* _context);

	void tick(float _delta);

private:
	void onAvailable();
	void onIng();
	void onCountDown();

private:
	FSkill m_param {};
	EnumUtil::EnumCallback<ECountDown> m_state { };
	class AShipPawn* m_pawn{ nullptr };

	TUniquePtr<class SkillBehaviour> m_behaviour {};

	float m_currentTime { 0.0f };
};
