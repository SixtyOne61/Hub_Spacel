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
	SkillBehaviour(class ACommonPawn* _pawn, ENetMode _netMode);
	virtual ~SkillBehaviour() {}

	virtual ESkillReturn onStart() { return ESkillReturn::Unavailable; }
	virtual void onEnd() = 0;
	virtual void onEndCountDown() = 0;

	template<class T>
	inline T* get() { return Cast<T>(m_pawn); }

	inline ACommonPawn* get() const { return m_pawn; }

protected:
	class ACommonPawn* m_pawn { nullptr };
	ENetMode m_netMode {};
};

class HUB_SPACEL_API SkillHealPack : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillHealPack() {}

	ESkillReturn onStart() override;
	void onEnd() override {};
	void onEndCountDown() override {};
};

class HUB_SPACEL_API SkillRepair : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillRepair() {}

	ESkillReturn onStart() override;
	void onEnd() override {};
	void onEndCountDown() override {};
};

class HUB_SPACEL_API SkillEscapeMode : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillEscapeMode() {}

	ESkillReturn onStart() override;
	void onEnd() override;
	void onEndCountDown() override;
};

class HUB_SPACEL_API SkillMissile : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillMissile() {}

	ESkillReturn onStart() override;
	void onEnd() override;
	void onEndCountDown() override {};
};

class HUB_SPACEL_API SkillFarmer : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillFarmer() {}

	ESkillReturn onStart() override;
	void onEnd() override;
	void onEndCountDown() override {};
};

class HUB_SPACEL_API SkillSpecialSupport : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillSpecialSupport() {}

	ESkillReturn onStart() override;
	void onEnd() override {};
	void onEndCountDown() override {};
};

class HUB_SPACEL_API SkillMetaFormAttack : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillMetaFormAttack() {}

	ESkillReturn onStart() override;
	void onEnd() override;
	void onEndCountDown() override {};
};

class HUB_SPACEL_API SkillMetaFormProtection : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillMetaFormProtection() {}

	ESkillReturn onStart() override;
	void onEnd() override;
	void onEndCountDown() override {};
};

class HUB_SPACEL_API SkillMetaFormSupport : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillMetaFormSupport() {}

	ESkillReturn onStart() override;
	void onEnd() override;
	void onEndCountDown() override {};
};

class HUB_SPACEL_API SkillKatyusha : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillKatyusha() {}

	ESkillReturn onStart() override;
	void onEnd() override {};
	void onEndCountDown() override {};
};

class HUB_SPACEL_API SkillEmergency : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillEmergency() {}

	ESkillReturn onStart() override;
	void onEnd() override {};
	void onEndCountDown() override;
};

class HUB_SPACEL_API SkillFactory
{
public:
	static TUniquePtr<SkillBehaviour> create(ESkill _skill, class ACommonPawn* _pawn, ENetMode _netMode);
};