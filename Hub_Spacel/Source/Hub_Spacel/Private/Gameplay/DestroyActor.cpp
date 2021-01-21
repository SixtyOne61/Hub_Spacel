// Fill out your copyright notice in the Description page of Project Settings.


#include "DestroyActor.h"
#include "Util/Tag.h"

void ADestroyActor::BeginPlay()
{
    Super::BeginPlay();
    this->Tags.Add(Tags::DestroyActor);
}