// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CommonPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API ACommonPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	/* override */
	void SetupInputComponent() override;
	void Tick(float _deltaTime) override;

protected:
	void hitResultUnderCursor(bool & _hasDeproj, bool & _hasHit, FVector & _out, FVector & _mouseWorldLocation, FVector & _mouseWorldDirection, TWeakObjectPtr<AActor>& _hit);
	bool getHitResultUnderCursor(ECollisionChannel TraceChannel, bool bTraceComplex, FHitResult& HitResult, AActor* _ignoreActor);

	template<class T>
	void updateMouseLocation(FVector const& _loc, FVector const& _dir, FVector const& _hitLoc);

protected:
	/* input callback */
	virtual void forward(float _value) {};
	virtual void horizontalStraf(float _value) {};
	virtual void verticalStraf(float _value) {};
	virtual void flightAttitude(float _value) {};
	virtual void fireOn() {};
	virtual void fireOff() {};
	virtual void returnToMainMenu() {};
	virtual void lock() {};
	virtual void skill(float _slot) {};
};

template<class T>
void ACommonPlayerController::updateMouseLocation(FVector const& _loc, FVector const& _dir, FVector const& _hitLoc)
{
	if (T* shipPawn = Cast<T>(this->GetPawn()))
	{
		shipPawn->lookAt(_loc, _dir, _hitLoc);
	}
}