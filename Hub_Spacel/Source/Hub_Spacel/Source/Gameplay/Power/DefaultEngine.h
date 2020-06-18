// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Source/ProceduralActor.h"
#include "DefaultEngine.generated.h"

UCLASS()
class HUB_SPACEL_API ADefaultEngine : public AProceduralActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADefaultEngine();

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    /* override */
    virtual bool GenerateMesh(TArray<FVector> const& _ignoreCoord) override;
};
