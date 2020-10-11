// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Interfaces/NetworkPredictionInterface.h"
#include "ShipPawnMovement.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UShipPawnMovement : public UPawnMovementComponent//, public INetworkPredictionInterface
{
	GENERATED_BODY()
	
public:
    UFUNCTION()
    void SetVelocityInLocalSpace(FVector const& _velocity);

protected:
    /* override */
    virtual void TickComponent(float _deltaTime, enum ELevelTick _tickType, FActorComponentTickFunction *_thisTickFunction) override;

private:
    UPROPERTY()
    FVector InputVelocity = FVector::ZeroVector;
};
