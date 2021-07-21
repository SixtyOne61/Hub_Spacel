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
	void RPCServerHorizontalStraf(float _val);

	UFUNCTION(Reliable, Server)
	void RPCServerVerticalStraf(float _val);

	UFUNCTION(Reliable, Server)
	void RPCServerFlightAttitude(float _val);

	UFUNCTION(Reliable, Server)
	void RPCServerFire(bool _is);

	UFUNCTION(Reliable, Server)
	void RPCServerStartGame();

	UFUNCTION(Reliable, Server)
	void RPCServerEndGame();

	/* input callback */
	void forward(float _value) override;
	void horizontalStraf(float _value) override;
	void verticalStraf(float _value) override;
	void flightAttitude(float _value) override;
	void fireOn() override;
	void fireOff() override;
	void lock() override;

	// TO DO : DEPRECATED change for something else
	void showScore();
	void hideScore();

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
