// Fill out your copyright notice in the Description page of Project Settings.


#include "SimpleBullet.h"

// Sets default values
ASimpleBullet::ASimpleBullet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SimpleBulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DefaultMeshComponent"));
	RootComponent = SimpleBulletMesh;
}

// Called when the game starts or when spawned
void ASimpleBullet::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASimpleBullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ASimpleBullet::launchBullet(FVector _forward)
{
	if (SimpleBulletMesh)
	{
		SimpleBulletMesh->AddForce(_forward * ForceOnStart);
	}
}

