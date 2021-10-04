// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Util/EmptyStruct.h"
#include "Player/Common/CommonPlayerController.h"
#include "GamePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API AGamePlayerController : public ACommonPlayerController
{
	GENERATED_BODY()
	
	friend class AShipPawn;

public:
	/* override */
	void BeginPlay() override;
	void Tick(float _deltaTime) override;
	void SetupInputComponent() override;

private:
	/* network */
	UFUNCTION(Reliable, Server)
	void RPCServerUpdateMouseLocation(FVector_NetQuantize100 const& _loc, FVector_NetQuantize100 const& _dir, FVector_NetQuantize100 const& _hitLoc);

	UFUNCTION(Reliable, Server)
	void RPCServerForward(float _val);

	UFUNCTION(Reliable, Server)
	void RPCServerBackward(float _val);

	UFUNCTION(Reliable, Server)
	void RPCServerHorizontalStrafRight(float _val);

	UFUNCTION(Reliable, Server)
	void RPCServerHorizontalStrafLeft(float _val);

	UFUNCTION(Reliable, Server)
	void RPCServerVerticalStraf(float _val);

	UFUNCTION(Reliable, Server)
	void RPCServerFlightAttitudeRight(float _val);

	UFUNCTION(Reliable, Server)
	void RPCServerFlightAttitudeLeft(float _val);

	UFUNCTION(Reliable, Server)
	void RPCServerFire(bool _is);

	UFUNCTION(Reliable, Server)
	void RPCServerStartGame();

	UFUNCTION(Reliable, Server)
	void RPCServerEndGame();

	UFUNCTION(Reliable, Server)
	void RPCServerHalfTurn();

	/* input callback */
	void forward(float _value) override;
	void backward(float _value) override;
	void horizontalStrafRight(float _value) override;
	void horizontalStrafLeft(float _value) override;
	void verticalStraf(float _value) override;
	void flightAttitudeRight(float _value) override;
	void flightAttitudeLeft(float _value) override;

	void fireOn() override;
	void fireOff() override;
	void lock() override;
	void halfTurn() override;

	// deprecated to do remove
	void showMission();
	void hideMission();

	/* from event */
	UFUNCTION()
	void GameModeChangeState(EGameState _state);

	void kill();
	void restarted();

	bool isAvailable() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	APawn* LinkPawn { nullptr };

private:
	UPROPERTY(Replicated)
	bool R_EnableInput { false };
};
