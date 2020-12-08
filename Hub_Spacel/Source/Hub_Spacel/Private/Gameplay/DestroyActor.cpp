// Fill out your copyright notice in the Description page of Project Settings.


#include "DestroyActor.h"

void ADestroyActor::BeginPlay()
{
    Super::BeginPlay();
    this->Tags.Add("DestroyActor");
}