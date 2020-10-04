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
    // call when input speed change
    virtual void speed(float _val);

    UFUNCTION(Reliable, Server)
    void RPCServerSetSpeed(float _val);

public:
    UPROPERTY(Category = "Speed", VisibleAnywhere, BlueprintReadOnly)
    float PercentSpeed = 0.0f;
};
