// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UPlayerDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(Category = "Movement", EditAnywhere, BlueprintReadWrite)
    float MaxForwardSpeed = 6000.0f;

    UPROPERTY(Category = "Movement", EditAnywhere, BlueprintReadWrite)
    float ReachTimeUpSpeed = 1.5f;

    UPROPERTY(Category = "Movement", EditAnywhere, BlueprintReadWrite)
    float ReachTimeDownSpeed = 0.75f;

    UPROPERTY(Category = "Movement", EditAnywhere, BlueprintReadWrite)
    float FlightAttitudeSpeed = 30.0f;

    UPROPERTY(Category = "Movement", EditAnywhere, BlueprintReadWrite)
    float TurnSpeed = 30.0f;

    UPROPERTY(Category = "Movement", EditAnywhere, BlueprintReadWrite)
    float UpSpeed = 40.0f;

    UPROPERTY(Category = "Fire", EditAnywhere, BlueprintReadOnly)
    TSubclassOf<class AActor> BulletClass;

    UPROPERTY(Category = "Fire", EditAnywhere, BlueprintReadWrite)
    float TimeBetweenFire = 0.3f;

    UPROPERTY(Category = "Fire", EditAnywhere, BlueprintReadWrite)
    float ReduceTimeBetweenFireWithLevel = 0.7f;
};
