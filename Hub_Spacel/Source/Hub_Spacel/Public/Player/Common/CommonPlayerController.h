// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Util/DelayValue.h"
#include "CommonPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API ACommonPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	struct TData
	{
		DelayValue m_forward{};
		float m_lastForwardInput{};
		DelayValue m_horizontalStraf{};
		float m_lastHorizontalStrafInput{};
		DelayValue m_verticalStraf{};
		float m_lastVerticalStrafInput{};
		DelayValue m_flightAttitude{};
		float m_lastFlightAttitudeInput{};

		inline void reset()
		{
			m_forward.reset();
			m_horizontalStraf.reset();
			m_verticalStraf.reset();
			m_flightAttitude.reset();
		}
	} m_data;
	
public:
	/* override */
	void SetupInputComponent() override;
	void Tick(float _deltaTime) override;

protected:
	void hitResultUnderCursor(bool & _hasDeproj, bool & _hasHit, FVector & _out, FVector & _mouseWorldLocation, FVector & _mouseWorldDirection, TWeakObjectPtr<AActor>& _hit);
	bool getHitResultUnderCursor(ECollisionChannel TraceChannel, bool bTraceComplex, FHitResult& HitResult, AActor* _ignoreActor);

	template<class T>
	void updateMouseLocation(FVector const& _loc, FVector const& _dir, FVector const& _hitLoc);

	template<class T>
	void updatePawnData(float _deltaTime);

protected:
	/* input callback */
	virtual void forward(float _value) {};
	virtual void horizontalStraf(float _value) {};
	virtual void verticalStraf(float _value) {};
	virtual void flightAttitude(float _value) {};
	virtual void fireOn() {};
	virtual void fireOff() {};
	virtual void returnToMainMenu();
	virtual void lock() {};
	virtual void halfTurn() {};
};

template<class T>
void ACommonPlayerController::updateMouseLocation(FVector const& _loc, FVector const& _dir, FVector const& _hitLoc)
{
	if (T* shipPawn = Cast<T>(this->GetPawn()))
	{
		shipPawn->lookAt(_loc, _dir, _hitLoc);
	}
}

template<class T>
void ACommonPlayerController::updatePawnData(float _deltaTime)
{
	if (T* shipPawn = Cast<T>(this->GetPawn()))
	{
		shipPawn->RU_PercentSpeed = m_data.m_forward.addValue(m_data.m_lastForwardInput, _deltaTime);
		shipPawn->PercentHorizontalStraf = m_data.m_horizontalStraf.addValue(m_data.m_lastHorizontalStrafInput, _deltaTime);
		shipPawn->PercentVerticalStraf = m_data.m_verticalStraf.addValue(m_data.m_lastVerticalStrafInput, _deltaTime);
		shipPawn->PercentFlightAttitude = m_data.m_flightAttitude.addValue(m_data.m_lastFlightAttitudeInput, _deltaTime);
	}
}