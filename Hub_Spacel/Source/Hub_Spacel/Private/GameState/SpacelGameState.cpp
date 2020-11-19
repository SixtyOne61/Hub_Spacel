// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacelGameState.h"
#include "Net/UnrealNetwork.h"

void ASpacelGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASpacelGameState, LatestEvent);
    DOREPLIFETIME(ASpacelGameState, WinningTeam);
}