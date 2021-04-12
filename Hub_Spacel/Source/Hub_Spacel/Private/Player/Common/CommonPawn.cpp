// Fill out your copyright notice in the Description page of Project Settings.


#include "CommonPawn.h"
#include "Util/SimplyMath.h"

// Sets default values
ACommonPawn::ACommonPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ACommonPawn::lookAt(FVector const& _loc, FVector const& _dir, FVector const& _hitLoc)
{
    FVector farpoint = _loc + _dir * 3000;
    FVector bigFar = _loc + _dir * 100000;
    this->TargetLocation = _hitLoc.IsNearlyZero() ? bigFar : _hitLoc;

    FRotator rotation = SimplyMath::MyLookRotation(bigFar, this->GetActorUpVector(), this->GetActorLocation());
    this->SetActorRotation(rotation);
}

// Called when the game starts or when spawned
void ACommonPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACommonPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


