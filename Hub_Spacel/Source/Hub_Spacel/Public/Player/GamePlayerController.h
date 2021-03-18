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
	void forward(float _value);
	void horizontalStraf(float _value);
	void verticalStraf(float _value);
	void flightAttitude(float _value);
	void fireOn();
	void fireOff();
	void returnToMainMenu();
	void lock();
	void showScore();
	void hideScore();
	void skill(float _slot);

	/* from event */
	UFUNCTION()
	void StartGame();

	void kill();
	void restarted();

	bool GetHitResultUnderCursor(ECollisionChannel TraceChannel, bool bTraceComplex, FHitResult& HitResult, AActor* _ignoreActor);

	UFUNCTION()
	void OnAddEffect(EEffect _effect);

	UFUNCTION()
	void OnRemoveEffect(EEffect _effect);

	UFUNCTION(Reliable, Server)
	void RPCServerPossess();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	APawn* LinkPawn { nullptr };

private:
	UPROPERTY(Replicated)
	bool R_EnableInput { false };

	UPROPERTY(Replicated)
	bool R_Emp { false };

	TData m_data {};
};
