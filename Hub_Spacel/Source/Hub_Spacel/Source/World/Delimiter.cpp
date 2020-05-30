// Fill out your copyright notice in the Description page of Project Settings.


#include "Delimiter.h"
#include "Components/BoxComponent.h"

// Sets default values
ADelimiter::ADelimiter()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
    RootComponent = BoxComponent;
}

// Called when the game starts or when spawned
void ADelimiter::BeginPlay()
{
    Super::BeginPlay();
    BoxComponent->OnComponentEndOverlap.AddDynamic(this, &ADelimiter::OnEndOverlap);
}

// Called every frame
void ADelimiter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADelimiter::OnEndOverlap(class UPrimitiveComponent* _overlappedComp, class AActor* _otherActor, class UPrimitiveComponent* _otherComp, int32 _otherBodyIndex)
{
    if (!_otherActor)
    {
        return;
    }

    // destroy all object leave delimiter (depend of config collision)
    // we don't generate overlap event for player for this moment
    _otherActor->Destroy();
}

