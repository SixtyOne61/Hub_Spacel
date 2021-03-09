// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enum/SpacelEnum.h"
#include <type_traits>

/**
 * 
 */
class HUB_SPACEL_API SkillBehaviour
{
public:
	SkillBehaviour(class AShipPawn* _pawn);
	virtual ~SkillBehaviour() {}

	virtual bool onStart() { return true; }
	virtual void onEnd() = 0;
	virtual void onEndCountDown() = 0;

protected:
	class AShipPawn* m_pawn { nullptr };
};

class HUB_SPACEL_API SkillRepairProtection : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillRepairProtection() {}

	bool onStart() override;
	void onEnd() override {};
	void onEndCountDown() override {};
};

class HUB_SPACEL_API SkillRepairSupport : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillRepairSupport() {}

	bool onStart() override;
	void onEnd() override {};
	void onEndCountDown() override {};
};

class HUB_SPACEL_API SkillGiveAlly1 : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillGiveAlly1() {}

	void onEnd() override {};
	void onEndCountDown() override {};
};

class HUB_SPACEL_API SkillGiveAlly2 : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillGiveAlly2() {}

	void onEnd() override {};
	void onEndCountDown() override {};
};

class HUB_SPACEL_API SkillEscapeMode : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillEscapeMode() {}

	bool onStart() override;
	void onEnd() override;
	void onEndCountDown() override;
};

class HUB_SPACEL_API SkillSpecialAttack : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillSpecialAttack() {}

	bool onStart() override;
	void onEnd() override {};
	void onEndCountDown() override {};
};

class HUB_SPACEL_API SkillSpecialProtection : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillSpecialProtection() {}

	void onEnd() override {};
	void onEndCountDown() override {};
};

class HUB_SPACEL_API SkillSpecialSupport : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillSpecialSupport() {}

	void onEnd() override {};
	void onEndCountDown() override {};
};

class HUB_SPACEL_API SkillFactory
{
public:
	static TUniquePtr<SkillBehaviour> create(ESkill _skill, class AShipPawn* _pawn);
};