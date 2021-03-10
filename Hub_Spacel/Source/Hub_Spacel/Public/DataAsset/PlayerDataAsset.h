// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DataAsset/InputDataAsset.h"
#include "PlayerDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UPlayerDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(Category = "Input", EditAnywhere, BlueprintReadWrite)
    UInputDataAsset* ForwardInput { nullptr };

    UPROPERTY(Category = "Input", EditAnywhere, BlueprintReadWrite)
    UInputDataAsset* HorizontalStrafInput { nullptr };

    UPROPERTY(Category = "Input", EditAnywhere, BlueprintReadWrite)
    UInputDataAsset* VerticalStrafInput { nullptr };

    UPROPERTY(Category = "Input", EditAnywhere, BlueprintReadWrite)
    UInputDataAsset* FlightAttitudeInput { nullptr };

    UPROPERTY(Category = "Movement", EditAnywhere, BlueprintReadWrite)
    float MaxForwardSpeed { 6000.0f };

    UPROPERTY(Category = "Movement", EditAnywhere, BlueprintReadWrite)
    float MaxHorizontalSpeed { 1000.0f };

    UPROPERTY(Category = "Movement", EditAnywhere, BlueprintReadWrite)
    float MaxVerticalSpeed { 1000.0f };

    UPROPERTY(Category = "Movement", EditAnywhere, BlueprintReadWrite)
    float MaxFlightAttitudeSpeed { 1000.0f };

    UPROPERTY(Category = "Escape Mode", EditAnywhere, BlueprintReadWrite)
    float EscapeModeDuration { 5.0f }; // deprecated

    UPROPERTY(Category = "Escape Mode", EditAnywhere, BlueprintReadWrite)
    float EscapeModeCountDown { 15.0f }; // deprecated

    UPROPERTY(Category = "Escape Mode", EditAnywhere, BlueprintReadWrite)
    float EscapeModeCoef { 1.5f };

    UPROPERTY(Category = "Fire", EditAnywhere, BlueprintReadOnly)
    TSubclassOf<class AActor> BulletClass;

    UPROPERTY(Category = "Fire", EditAnywhere, BlueprintReadOnly)
    TSubclassOf<class AActor> MissileClass;

    UPROPERTY(Category = "Fire", EditAnywhere, BlueprintReadWrite)
    float TimeBetweenFire = 0.3f;

    UPROPERTY(Category = "Fire", EditAnywhere, BlueprintReadWrite)
    float ReduceTimeBetweenFireWithLevel = 0.7f;
};
