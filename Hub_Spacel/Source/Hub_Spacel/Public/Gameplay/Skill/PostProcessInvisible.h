// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Enum/SpacelEnum.h"
#include "PostProcessInvisible.generated.h"

UCLASS()
class HUB_SPACEL_API APostProcessInvisible : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APostProcessInvisible();

	UFUNCTION()
	void OnRemoveEffect(EEffect _effect);
};
