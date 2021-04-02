// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "World/BillboardActor.h"
#include "TargetActor.generated.h"

UCLASS()
class HUB_SPACEL_API ATargetActor : public ABillboardActor
{
	GENERATED_BODY()
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	void showTarget(bool _show);
};
