// Fill out your copyright notice in the Description page of Project Settings.


#include "NinePackActor.h"

// Sets default values
ANinePackActor::ANinePackActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ANinePackActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANinePackActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

