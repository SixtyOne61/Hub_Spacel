// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Source/Gameplay/Power/DefaultEngine.h"
#include "SmallEngine.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API ASmallEngine : public ADefaultEngine
{
	GENERATED_BODY()

public:
    //UFUNCTION(BlueprintCallable) -> override, no need
    virtual bool GenerateMesh(TArray<FVector> const& _ignoreCoord) override;
	
};
