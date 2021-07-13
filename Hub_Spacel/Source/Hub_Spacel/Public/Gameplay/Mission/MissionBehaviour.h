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
	MissionBehaviour(FMission const& _mission) : m_mission (_mission) { };
	virtual ~MissionBehaviour() {};

	virtual void start(class UWorld* _world) { m_isStart = true; };
	virtual void tick(float _deltaTime, UWorld* _world)
	{
		if (!m_isStart)
		{
			start(_world);
		}
		else
		{
			if (m_mission.DurationValue != 0.0f)
			{
				m_timer += _deltaTime;
				if (m_timer >= m_mission.DurationValue)
				{
					end();
				}
			}
		}
	}
	virtual void end() { m_isEnd = true; };

	void resetTimer() { m_timer = 0.0f; }
	inline bool isEnd() const { return m_isEnd; }

protected:
	FMission m_mission {};

private:
	bool m_isStart{ false };
	bool m_isEnd { false };
	float m_timer { 0.0f };
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
	using MissionBehaviour::MissionBehaviour;
};

class HUB_SPACEL_API MissionEcartType : public MissionSilence
{
	using MissionSilence::MissionSilence;

public:
	void start(UWorld* _world) override;
	void tick(float _deltaTime, UWorld* _world) override;

private:
	void onKill(FString const& _victim, FString const& _killer);

private:
	FString m_loosingTeam {};
	bool m_killDone { false };
};

class HUB_SPACEL_API MissionComet : public MissionBehaviour
{
	// TO DO override start and spawn comet on start
	using MissionBehaviour::MissionBehaviour;

public:
	void start(class UWorld* _world) override;
	void tick(float _deltaTime, UWorld* _world) override;

	void onCometDestroy(FString const& _team);

private:
	int m_nbComet { 0 };
	TSet<FString> m_teams {};
};

class HUB_SPACEL_API MissionPirate : public MissionBehaviour
{
	using MissionBehaviour::MissionBehaviour;

public:
	void start(class UWorld* _world) override;
	void tick(float _deltaTime, UWorld* _world) override;

	void onKill(FName const& _team);

private:
	FName m_team { "" };
};

class HUB_SPACEL_API MissionTakeGold : public MissionBehaviour
{
	using MissionBehaviour::MissionBehaviour;

public:
	void start(class UWorld* _world) override;
	void tick(float _deltaTime, UWorld* _world) override;

	void onTaken(uint32 const& _id);

private:
	uint32 m_id {};
};

class HUB_SPACEL_API MissionHoldGold : public MissionBehaviour
{
	using MissionBehaviour::MissionBehaviour;

public:
	void start(class UWorld* _world) override;
	void tick(float _deltaTime, UWorld* _world) override;
	void findGold(class UWorld* _world);

private:
	void onTokenChange();

private:
	bool m_find { false };
};