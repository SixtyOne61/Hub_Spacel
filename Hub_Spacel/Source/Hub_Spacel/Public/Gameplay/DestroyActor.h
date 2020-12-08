// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DestroyActor.generated.h"

UCLASS()
class HUB_SPACEL_API ADestroyActor : public AActor
{
	GENERATED_BODY()
	
public:	
	virtual void hit(FHitResult const& _info) {};
};
