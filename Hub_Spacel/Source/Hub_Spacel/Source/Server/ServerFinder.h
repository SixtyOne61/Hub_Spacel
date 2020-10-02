// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ServerDesc.h"

/**
 * 
 */
class HUB_SPACEL_API ServerFinder
{
public:
	ServerFinder();
	~ServerFinder();

	UFUNCTION(BlueprintCallable)
	inline TArray<FServerDesc> const& GetServers() const { return m_servers; }

    UFUNCTION(BlueprintCallable)
    void CleanServers();

private:
	// server list
	TArray<FServerDesc> m_servers = { { } };
};
