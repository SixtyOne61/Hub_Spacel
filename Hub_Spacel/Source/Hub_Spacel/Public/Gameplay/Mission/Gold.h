// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Mission/MissionActor.h"
#include "Util/SpacelEvent.h"
#include "Gold.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API AGold : public AMissionActor
{
	GENERATED_BODY()
	
	using base_type = AMissionActor;

public:
	void hit(class AShipPawn* _pawn, FString const& _team) override;

public:
	Util::Event<uint32 const&> OnDestroyDelegate{ };
};
