// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Source/ProceduralActor.h"
#include "DefaultShell.generated.h"

UCLASS()
class HUB_SPACEL_API ADefaultShell : public AProceduralActor
{
	GENERATED_BODY()
	
public:
    ADefaultShell();

public:
    virtual bool GenerateMesh(TArray<FVector> const& _ignoreCoord) override;
};
