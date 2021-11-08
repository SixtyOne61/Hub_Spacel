// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Enum/SpacelEnum.h"
#include "FormDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UFormDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	EFormType Type {};

	UPROPERTY(EditAnywhere)
	FString Path {};

	UPROPERTY(EditAnywhere)
	class UStaticMesh* StaticMesh { nullptr };
};
