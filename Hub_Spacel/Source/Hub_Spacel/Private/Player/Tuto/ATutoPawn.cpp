// Fill out your copyright notice in the Description page of Project Settings.


#include "ATutoPawn.h"

// Sets default values
AATutoPawn::AATutoPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AATutoPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AATutoPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AATutoPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

