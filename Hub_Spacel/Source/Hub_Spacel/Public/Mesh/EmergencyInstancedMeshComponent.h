// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Mesh/SpacelInstancedMeshComponent.h"
#include "Player/ShipPawn.h"
#include "EmergencyInstancedMeshComponent.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UEmergencyInstancedMeshComponent : public USpacelInstancedMeshComponent
{
	GENERATED_BODY()
	
	inline int Remove(FVector_NetQuantize const& _location) override
	{
		auto ret = Super::Remove(_location);

		if (AShipPawn* pawn = Cast<AShipPawn>(this->GetOwner()))
		{
			pawn->emergencyRedCube();
		}
		return ret;
	}
};
