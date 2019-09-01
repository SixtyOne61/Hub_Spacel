// Fill out your copyright notice in the Description page of Project Settings.


#include "Asteroid.h"
#include "Hub_Spacel/Source/Mesh/SpacelProceduralMeshComponent.h"

// Sets default values
AAsteroid::AAsteroid()
	: m_proceduralMeshComponent(nullptr)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	m_proceduralMeshComponent = CreateDefaultSubobject<USpacelProceduralMeshComponent>(TEXT("Procedural mesh"));
	m_proceduralMeshComponent->bUseAsyncCooking = true;
	RootComponent = m_proceduralMeshComponent;
}

// Called when the game starts or when spawned
void AAsteroid::BeginPlay()
{
	Super::BeginPlay();

	if(m_proceduralMeshComponent)
	{
		m_proceduralMeshComponent->setCubeSize(100.0f);
		m_proceduralMeshComponent->generateMesh();
	}
}

// Called every frame
void AAsteroid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

