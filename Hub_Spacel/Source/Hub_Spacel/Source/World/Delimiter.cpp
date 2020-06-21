// Fill out your copyright notice in the Description page of Project Settings.


#include "Delimiter.h"
#include "Components/BoxComponent.h"

// Sets default values
ADelimiter::ADelimiter()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
    if (!ensure(BoxComponent != nullptr)) return;
    RootComponent = BoxComponent;
}

// Called when the game starts or when spawned
void ADelimiter::BeginPlay()
{
    Super::BeginPlay();
    if (this->HasAuthority())
    {
        if (!ensure(this->BoxComponent != nullptr)) return;
        this->BoxComponent->OnComponentEndOverlap.AddDynamic(this, &ADelimiter::OnEndOverlap);
    }
}

void ADelimiter::OnEndOverlap(class UPrimitiveComponent* _overlappedComp, class AActor* _otherActor, class UPrimitiveComponent* _otherComp, int32 _otherBodyIndex)
{
    if (!ensure(_otherActor != nullptr)) return;

    // destroy all object leave delimiter (depend of config collision)
    // we don't generate overlap event for player for this moment
    _otherActor->Destroy();
}

