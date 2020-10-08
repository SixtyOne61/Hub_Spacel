// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerShipController.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API APlayerShipController : public APlayerController
{
	GENERATED_BODY()

public:
    virtual void SetupInputComponent() override;
	
protected:
    /* call when input speed change */
    virtual void speed(float _val);

    UFUNCTION(Reliable, Server)
    void RPCServerSetSpeed(float _val);

    /* call when input turn change */
    virtual void turn(float _val);

    UFUNCTION(Reliable, Server)
    void RPCServerSetTurn(float _val);

    /* call when input flight attitude change */
    virtual void flightAttitude(float _val);

    UFUNCTION(Reliable, Server)
    void RPCServerSetFlightAttitude(float _val);

    /* call when input up change */
    virtual void up(float _val);

    UFUNCTION(Reliable, Server)
    void RPCServerSetUp(float _val);

public:
    UPROPERTY(Category = "Speed", VisibleAnywhere, BlueprintReadOnly)
    float PercentSpeed = 0.0f;
};
