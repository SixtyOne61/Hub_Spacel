// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacelPlayerState.h"
#include "Net/UnrealNetwork.h"

void ASpacelPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASpacelPlayerState, Team);
}
