// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataAsset/MissionDataAsset.h"
#include "Util/SpacelEvent.h"
/**
 * 
 */
class HUB_SPACEL_API MissionBehaviour
{
	friend class AMissionManager;

public:
	MissionBehaviour(FMission const& _mission) : m_mission (_mission) {};
	virtual ~MissionBehaviour() {};

	virtual void tick(float _deltaTime, UWorld* _world) = 0;
	virtual void end() { m_isEnd = true; };

protected:
	FMission m_mission {};
	bool m_isEnd { false };
};

class HUB_SPACEL_API MissionFirstBlood : public MissionBehaviour
{
	using MissionBehaviour::MissionBehaviour;

public:
	void tick(float _deltaTime, UWorld* _world) override;
};

class HUB_SPACEL_API MissionRaceScore : public MissionBehaviour
{
	using MissionBehaviour::MissionBehaviour;

public:
	void tick(float _deltaTime, UWorld* _world) override;
};

class HUB_SPACEL_API MissionSilence : public MissionBehaviour
{
	friend class AMissionManager;

public:
	MissionSilence(FMission const& _mission) : MissionBehaviour(_mission) {};

protected:
	bool m_start { false };
};

class HUB_SPACEL_API MissionEcartType : public MissionSilence
{
	using MissionSilence::MissionSilence;

public:
	void tick(float _deltaTime, UWorld* _world) override;

private:
	void onKill(FString const& _victim, FString const& _killer);

private:
	FString m_loosingTeam {};
	bool m_killDone { false };
	float m_timer { 0.0f };
};

class HUB_SPACEL_API MissionComet : public MissionBehaviour
{
	using MissionBehaviour::MissionBehaviour;

public:
	void tick(float _deltaTime, UWorld* _world) override;

	void onCometDestroy();
};