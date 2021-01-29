// Fill out your copyright notice in the Description page of Project Settings.


#include "DestroyActor.h"
#include "Util/Tag.h"

void ADestroyActor::BeginPlay()
{
    Super::BeginPlay();
    this->Tags.Add(Tags::DestroyActor);
}

void ADestroyActor::applyHit(TArray<int32>& _instance)
{
    if (this->GetNetMode() != ENetMode::NM_DedicatedServer)
    {
        // conception error
        ensure(false);
    }
}