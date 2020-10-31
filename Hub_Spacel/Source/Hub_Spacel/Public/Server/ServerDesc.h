// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enum/SpacelEnum.h"
#include "ServerDesc.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FServerDesc
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERegionType Region = ERegionType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name = "Default Server Name";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString IP = "127.0.0.1";

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int NbClient = 18;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Ping = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool Status = true;
};
