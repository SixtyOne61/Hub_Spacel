// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Util/EmptyStruct.h"
#include "Util/DelayValue.h"
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

		inline void reset()
		{
			m_forward.reset();
			m_horizontalStraf.reset();
			m_verticalStraf.reset();
			m_flightAttitude.reset();
		}
	};

public:
	/* override */
	void BeginPlay() override;
	void Tick(float _deltaTime) override;
	void SetupInputComponent() override;

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
	void RPCServerFire(bool _is);

	UFUNCTION(Reliable, Server)
	void RPCServerSkill(float _slot);

	UFUNCTION(Reliable, Server)
	void RPCServerStartGame();

	/* input callback */
	void forward(float _value) override;
	void horizontalStraf(float _value) override;
	void verticalStraf(float _value) override;
	void flightAttitude(float _value) override;
	void fireOn() override;
	void fireOff() override;
	void returnToMainMenu() override;
	void lock() override;
	void skill(float _slot) override;

	void showScore();
	void hideScore();

	/* from event */
	UFUNCTION()
	void StartGame();

	void kill();
	void restarted();

	UFUNCTION()
	void OnAddEffect(EEffect _effect);

	UFUNCTION()
	void OnRemoveEffect(EEffect _effect);

	UFUNCTION(Reliable, Server)
	void RPCServerPossess();

	bool isAvailable() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	APawn* LinkPawn { nullptr };

private:
	UPROPERTY(Replicated)
	bool R_EnableInput { false };

	TData m_data {};
};
