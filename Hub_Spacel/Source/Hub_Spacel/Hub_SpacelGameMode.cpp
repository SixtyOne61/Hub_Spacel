// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Hub_SpacelGameMode.h"
#include "Hub_SpacelPawn.h"

AHub_SpacelGameMode::AHub_SpacelGameMode()
{
	// set default pawn class to our flying pawn
	DefaultPawnClass = AHub_SpacelPawn::StaticClass();
}
