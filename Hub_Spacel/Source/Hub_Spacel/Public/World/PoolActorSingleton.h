// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

class HUB_SPACEL_API Empty {};

template<class T>
class HUB_SPACEL_API Singleton : public Empty
{
protected:
	Singleton() {};
	virtual ~Singleton() {};

public:
	Singleton<T>(Singleton<T>&) = delete;
	void operator=(Singleton<T> const&) = delete;

	static T& Get()
	{
		static T instance = {};
		return instance;
	}
};

template<class T>
class HUB_SPACEL_API ActorSingleton : public Singleton<ActorSingleton<T>>
{
	friend class Singleton<ActorSingleton<T>>;
private:
	ActorSingleton() {};

public:
	inline void add(T* _actor) { m_actors.Add(_actor); }

	inline T* getActor()
	{
		if (m_actors.Num())
		{
			return m_actors.Pop();
		}
		return nullptr;
	}

public:
	TArray<T*> m_actors {};
};

class HUB_SPACEL_API BulletSingleton : public ActorSingleton<class ALaserBullet>
{
};

class HUB_SPACEL_API MissileSingleton : public ActorSingleton<class AMissile>
{
};

/**
 * 
 */
class HUB_SPACEL_API PoolActorSingleton : public Singleton<PoolActorSingleton>
{
	friend class Singleton<PoolActorSingleton>;
private:
	PoolActorSingleton() {};

public:
	template<class T>
	void RegisterActor(T* _actor);

	template<class T>
	T* GetActor();
};

template<class T>
void PoolActorSingleton::RegisterActor(T* _actor)
{
	static_assert(false, "Register Actor was not generate for this type\n.");
}

template<>
inline void PoolActorSingleton::RegisterActor(class ALaserBullet* _actor)
{
	BulletSingleton::Get().add(_actor);
}

template<>
inline void PoolActorSingleton::RegisterActor(class AMissile* _actor)
{
	MissileSingleton::Get().add(_actor);
}

template<class T>
T* PoolActorSingleton::GetActor()
{
	static_assert(false, "Get Actor was not generate for this type\n.");
}

template<>
inline ALaserBullet* PoolActorSingleton::GetActor()
{
	return BulletSingleton::Get().getActor();
}

template<>
inline AMissile* PoolActorSingleton::GetActor()
{
	return MissileSingleton::Get().getActor();
}