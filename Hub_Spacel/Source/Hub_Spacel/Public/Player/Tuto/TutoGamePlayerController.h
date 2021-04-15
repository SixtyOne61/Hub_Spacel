// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/Common/CommonPlayerController.h"
#include "TutoGamePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API ATutoGamePlayerController : public ACommonPlayerController
{
	GENERATED_BODY()
	
public:
	/* override */
	void Tick(float _deltaTime) override;

protected:
	/* input callback */
	void forward(float _value) override;
	void horizontalStraf(float _value) override;
	void verticalStraf(float _value) override;
	void flightAttitude(float _value) override;
	void fireOn() override;
	void fireOff() override;
	void lock() override;
	void skill(float _slot) override;
};