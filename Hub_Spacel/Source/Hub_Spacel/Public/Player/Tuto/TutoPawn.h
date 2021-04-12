// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/Common/CommonPawn.h"
#include "TutoPawn.generated.h"

UCLASS()
class HUB_SPACEL_API ATutoPawn : public ACommonPawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ATutoPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
