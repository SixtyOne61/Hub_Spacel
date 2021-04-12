// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CommonPawn.generated.h"

UCLASS()
class HUB_SPACEL_API ACommonPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACommonPawn();

	void lookAt(FVector const& _loc, FVector const& _dir, FVector const& _hitLoc);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	FVector TargetLocation{ FVector::ZeroVector };
};
