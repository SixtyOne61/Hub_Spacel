// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Util/EmptyStruct.h"
#include "Util/DelayValue.h"
#include "GamePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API AGamePlayerController : public APlayerController
{
	GENERATED_BODY()
	
	struct TData
	{
		DelayValue m_forward {};
		float m_lastForwardInput {};
		DelayValue m_horizontalStraf {};
		float m_lastHorizontalStrafInput {};
		DelayValue m_verticalStraf {};
		float m_lastVerticalStrafInput {};
		DelayValue m_flightAttitude {};
		float m_lastFlightAttitudeInput {};
	};

public:
	/* override */
	void BeginPlay() override;
	void Tick(float _deltaTime) override;
	void SetupInputComponent() override;

	/* restart player */
	void Restart();

private:
	/* network */
	UFUNCTION(Reliable, Server)
	void RPCServerUpdateMouseLocation(FVector const& _loc, FVector const& _dir, FVector const& _hitLoc);

	UFUNCTION(Reliable, Server)
	void RPCServerForward(float _val);

	UFUNCTION(Reliable, Server)
	void RPCServerHorizontalStraf(float _val);

	UFUNCTION(Reliable, Server)
	void RPCServerVerticalStraf(float _val);

	UFUNCTION(Reliable, Server)
	void RPCServerFlightAttitude(float _val);

	UFUNCTION(Reliable, Server)
	void RPCServerTriggerEscapeMode();

	UFUNCTION(Reliable, Server)
	void RPCServerFire(bool _is);

	UFUNCTION(Reliable, Server)
	void RPCServerRepairProtection();

	UFUNCTION(Reliable, Server)
	void RPCServerRepairSupport();

	UFUNCTION(Reliable, Server)
	void RPCServerStartGame();

	/* input callback */
	void forward(float _value);
	void horizontalStraf(float _value);
	void verticalStraf(float _value);
	void flightAttitude(float _value);
	void triggerEscapeMode();
	void fireOn();
	void fireOff();
	void returnToMainMenu();
	void repairProtection();
	void repairSupport();
	void giveAlly1();
	void giveAlly2();

	/* from event */
	UFUNCTION()
	void StartGame();

private:
	UPROPERTY(Replicated)
	bool R_EnableInput { false };

	TData m_data {};
};
