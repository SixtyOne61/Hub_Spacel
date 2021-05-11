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

class HUB_SPACEL_API SkillNinePack : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillNinePack() {}

	ESkillReturn onStart() override;
	void onEnd() override {};
	void onEndCountDown() override {};
};

class HUB_SPACEL_API SkillRepairProtection : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillRepairProtection() {}

	ESkillReturn onStart() override;
	void onEnd() override {};
	void onEndCountDown() override {};
};

class HUB_SPACEL_API SkillRepairSupport : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillRepairSupport() {}

	ESkillReturn onStart() override;
	void onEnd() override {};
	void onEndCountDown() override {};
};

class HUB_SPACEL_API SkillGiveAlly1 : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillGiveAlly1() {}

	ESkillReturn onStart() override;
	void onEnd() override {};
	void onEndCountDown() override {};
};

class HUB_SPACEL_API SkillGiveAlly2 : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillGiveAlly2() {}

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

class HUB_SPACEL_API SkillSpecialAttack : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillSpecialAttack() {}

	ESkillReturn onStart() override;
	void onEnd() override {};
	void onEndCountDown() override {};
};

class HUB_SPACEL_API SkillSpecialProtection : public SkillBehaviour
{
	using SkillBehaviour::SkillBehaviour;
public:
	virtual ~SkillSpecialProtection() {}

	ESkillReturn onStart() override;
	void onEnd() override;
	void onEndCountDown() override {};

private:
	void fillPlayer(FName const& _team, TArray<class AShipPawn*>& _pawns) const;
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

class HUB_SPACEL_API SkillFactory
{
public:
	static TUniquePtr<SkillBehaviour> create(ESkill _skill, class ACommonPawn* _pawn, ENetMode _netMode);
};