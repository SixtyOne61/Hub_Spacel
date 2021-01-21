// Fill out your copyright notice in the Description page of Project Settings.


#include "FogActor.h"
#include "NiagaraComponent.h"
#include "Components/BoxComponent.h"
#include "Player/ShipPawn.h"

// Sets default values
AFogActor::AFogActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	if (!ensure(BoxComponent != nullptr)) return;
	RootComponent = BoxComponent;

	FogNiagaraSystem = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	if (!ensure(FogNiagaraSystem != nullptr)) return;
	FogNiagaraSystem->SetupAttachment(RootComponent);

	Tags.Add("Fog");
}

// Called when the game starts or when spawned
void AFogActor::BeginPlay()
{
	Super::BeginPlay();

	if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		if (this->BoxComponent != nullptr)
		{
			this->BoxComponent->OnComponentBeginOverlap.AddDynamic(this, &AFogActor::OnBeginOverlap);
			this->BoxComponent->OnComponentEndOverlap.AddDynamic(this, &AFogActor::OnEndOverlap);
		}
	}
}

void AFogActor::onOverlap(class AActor* _otherActor, bool _value) const
{
	if (_otherActor != nullptr && _otherActor->ActorHasTag("Player"))
	{
		if (AShipPawn* shipPawn = Cast<AShipPawn>(_otherActor))
		{
			shipPawn->setIsInFog(_value);
		}
	}
}

void AFogActor::OnBeginOverlap(class UPrimitiveComponent* _comp, class AActor* _otherActor, class UPrimitiveComponent* _otherComp, int32 _otherBodyIndex, bool _bFromSweep, const FHitResult& _sweepResult)
{
	onOverlap(_otherActor, true);
	UE_LOG(LogTemp, Warning, TEXT("Begin Overlap"));
}

void AFogActor::OnEndOverlap(class UPrimitiveComponent* _comp, class AActor* _otherActor, class UPrimitiveComponent* _otherComp, int32 _otherBodyIndex)
{
	onOverlap(_otherActor, false);
	UE_LOG(LogTemp, Warning, TEXT("End Overlap"));
}
