// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacelPawn.h"

// Sets default values
ASpacelPawn::ASpacelPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASpacelPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASpacelPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASpacelPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

