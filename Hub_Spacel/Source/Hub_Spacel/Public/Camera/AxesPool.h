// All rights reserved Zubov Alexander zubetto85@gmail.com 2019

#pragma once

#include "CoreMinimal.h"
#include "AxesPool.generated.h"

/**
	Axes input values
 */
USTRUCT(BlueprintType)
struct FAxesPool
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Axes Input")
	float Distance;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Axes Input")
	float StickYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Axes Input")
	float StickPitch;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Axes Input")
	float CameraYaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Axes Input")
	float CameraPitch;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Axes Input")
	float DeltaTime;
};

USTRUCT(BlueprintType)
struct FDistanceRotator
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera Relative Position")
	float Distance;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera Relative Position")
	FRotator Rotation;
};


