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

    UPROPERTY(Category = "Movement", EditAnywhere, BlueprintReadWrite)
    float MinCoefSpeed { 0.6f };

    UPROPERTY(Category = "Movement", EditAnywhere, BlueprintReadWrite)
    float CoefOverDriveValue { 1.0f };

    UPROPERTY(Category = "Skill", EditAnywhere, BlueprintReadWrite)
    float EscapeModeCoef { 1.5f };

    UPROPERTY(Category = "Skill", EditAnywhere, BlueprintReadWrite)
    int32 ShieldLife { 50 };

    UPROPERTY(Category = "Fire", EditAnywhere, BlueprintReadOnly)
    TSubclassOf<class AActor> BulletClass;

    UPROPERTY(Category = "Fire", EditAnywhere, BlueprintReadOnly)
    TSubclassOf<class AActor> MissileClass;

    UPROPERTY(Category = "Fire", EditAnywhere, BlueprintReadOnly)
    TSubclassOf<class AActor> KatyushaClass;

    UPROPERTY(Category = "Fire", EditAnywhere, BlueprintReadWrite)
    float TimeBetweenFire = 0.3f;

    UPROPERTY(Category = "Skill", EditAnywhere, BlueprintReadWrite)
    float RepairRatio { 0.5f };

    UPROPERTY(Category = "Skill", EditAnywhere, BlueprintReadOnly)
    TSubclassOf<class AActor> EmpClass { nullptr };

    UPROPERTY(Category = "Skill", EditAnywhere, BlueprintReadWrite)
    TSubclassOf<class AActor> HealPackClass { nullptr };

    UPROPERTY(Category = "Skill", EditAnywhere, BlueprintReadOnly)
    TSubclassOf<class AActor> MetaSupportPostProcessClass;

    UPROPERTY(Category = "Skill", EditAnywhere, BlueprintReadOnly)
    TSubclassOf<class AActor> EmpPostProcessClass;

    UPROPERTY(Category = "Skill", EditAnywhere, BlueprintReadOnly)
    uint8 TresholdForEmergency { 2 };

    UPROPERTY(Category = "Skill", EditAnywhere, BlueprintReadOnly)
    uint8 TresholdForSwapEmergencyPercent { 10 };

    UPROPERTY(Category = "Farm", EditAnywhere, BlueprintReadWrite)
    int NbAsteroideForMatiere{};
};
