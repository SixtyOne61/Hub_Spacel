// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TutoGamePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API ATutoGamePlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	/* override */
	void SetupInputComponent() override;

	/* input callback */
	void forward(float _value);
	void horizontalStraf(float _value);
	void verticalStraf(float _value);
	void flightAttitude(float _value);
	void fireOn();
	void fireOff();
	void returnToMainMenu();
	void lock();
	void skill(float _slot);
};
