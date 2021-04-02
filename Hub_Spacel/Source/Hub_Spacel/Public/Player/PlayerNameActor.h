// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "World/BillboardActor.h"
#include "Styling/SlateColor.h"
#include "PlayerNameActor.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API APlayerNameActor : public ABillboardActor
{
	GENERATED_BODY()

public:
	void setPlayerName(FString const& _name, FSlateColor const& _teamColor);
};
