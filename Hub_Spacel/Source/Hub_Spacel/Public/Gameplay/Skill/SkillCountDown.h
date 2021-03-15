// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAsset/SkillDataAsset.h"
#include "Enum/SpacelEnum.h"
#include "Util/EnumUtil.h"
#include "Engine/EngineBaseTypes.h"

/**
 * 
 */
class HUB_SPACEL_API SkillCountDown
{
public:
	SkillCountDown(const SkillCountDown &);
	SkillCountDown(FSkill _skill, class AShipPawn* _pawn, ENetMode _netMode);
	~SkillCountDown();

	void use(class UWorld* _context);
	void tick(float _delta);
	inline FSkill const& getParam() const { return m_param; }
	void addProgressBar(class UProgressBar* _progressBar);

private:
	void onAvailable();
	void onIng();
	void onCountDown();
	void updatePercent(UProgressBar* _progressBar, float _percent);

private:
	ENetMode m_netMode {};
	FSkill m_param {};
	EnumUtil::EnumCallback<ECountDown> m_state { };
	class AShipPawn* m_pawn { nullptr };
	class UProgressBar* m_progressBar { nullptr };
	float m_currentTime{ 0.0f };

	TUniquePtr<class SkillBehaviour> m_behaviour {};
};
