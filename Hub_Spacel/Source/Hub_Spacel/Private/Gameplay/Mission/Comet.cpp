// Fill out your copyright notice in the Description page of Project Settings.


#include "Comet.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Util/Tag.h"
#include "Player/ShipPawn.h"

// Sets default values
AComet::AComet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = MeshComponent;

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
}

// Called when the game starts or when spawned
void AComet::BeginPlay()
{
	Super::BeginPlay();

	m_speedRoll = FMath::RandRange(10, 60);
	m_speedRoll *= FMath::RandRange(-1, 1);
}

void AComet::Tick(float _deltaTime)
{
	Super::Tick(_deltaTime);

	FRotator rot = this->GetActorRotation();
	rot.Add(0.0f, 0.0f, m_speedRoll * _deltaTime);
	this->SetActorRotation(rot);
}

void AComet::hit(AShipPawn * _pawn, FString const& _team)
{
	this->Destroy();
	if (_pawn != nullptr)
	{
		if (!_pawn->canTank(1))
		{
			_pawn->kill();
		}
		OnIntercepDelegate.broadcast(_team);
	}
}
