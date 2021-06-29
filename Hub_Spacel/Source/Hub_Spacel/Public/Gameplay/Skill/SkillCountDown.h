// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enum/SpacelEnum.h"
#include "Util/EnumUtil.h"
#include "Engine/EngineBaseTypes.h"
#include <functional>

/**
 * 
 */
class HUB_SPACEL_API SkillCountDown
{
public:
	SkillCountDown(const SkillCountDown &);
	SkillCountDown(class UUniqueSkillDataAsset const* _skill, class ACommonPawn* _pawn, ENetMode _netMode, std::function<void(ESkill)> _callbackSucced, std::function<void(ESkill, ESkillReturn)> _callbackFailed);
	~SkillCountDown();

	void use(class UWorld* _context);
	void tick(float _delta);
	inline class UUniqueSkillDataAsset const* getParam() const { return m_param; }
	void addProgressBar(class UProgressBar* _progressBar);

	bool& inputeState() { return m_isDown; }

	ESkill getSkillType() const;

private:
	void onAvailable();
	void onIng();
	void onCountDown();
	void updatePercent(UProgressBar* _progressBar, float _percent);

private:
	ENetMode m_netMode {};
	class UUniqueSkillDataAsset const* m_param { nullptr };
	EnumUtil::EnumCallback<ECountDown> m_state { };
	class ACommonPawn* m_pawn { nullptr };
	class UProgressBar* m_progressBar { nullptr };
	float m_currentTime{ 0.0f };

	TUniquePtr<class SkillBehaviour> m_behaviour {};

	bool m_isDown { false };
	std::function<void(ESkill)> m_callbackSucced {};
	std::function<void(ESkill, ESkillReturn)> m_callbackFailed{};
};
