// Fill out your copyright notice in the Description page of Project Settings.


#include "FogActor.h"
#include "NiagaraComponent.h"

// Sets default values
AFogActor::AFogActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	FogNiagaraSystem = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	if (!ensure(FogNiagaraSystem != nullptr)) return;
	RootComponent = FogNiagaraSystem;
}

// Called when the game starts or when spawned
void AFogActor::BeginPlay()
{
	Super::BeginPlay();
}

