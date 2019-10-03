// Fill out your copyright notice in the Description page of Project Settings.


#include "Asteroid.h"
#include "Hub_Spacel/Source/Mesh/SpacelProceduralMeshComponent.h"
#include "Hub_Spacel/Source/Projectile/SimpleBullet.h"

// Sets default values
AAsteroid::AAsteroid()
	: m_proceduralMeshComponent(nullptr)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	m_proceduralMeshComponent = CreateDefaultSubobject<USpacelProceduralMeshComponent>(TEXT("Procedural mesh"));
	m_proceduralMeshComponent->bUseAsyncCooking = true;
	RootComponent = m_proceduralMeshComponent;
}

void AAsteroid::setEdges(TArray<TSharedPtr<ChainedLocation>>&& _edges)
{
	if (!m_proceduralMeshComponent)
	{
		return;
	}

	m_proceduralMeshComponent->setEdges(std::forward<TArray<TSharedPtr<ChainedLocation>>>(_edges));
}

void AAsteroid::setCubeSize(int _cubeSize)
{
	if (!m_proceduralMeshComponent)
	{
		return;
	}

	m_proceduralMeshComponent->setCubeSize(_cubeSize);
}

// Called when the game starts or when spawned
void AAsteroid::BeginPlay()
{
	Super::BeginPlay();

	if(m_proceduralMeshComponent)
	{
		m_proceduralMeshComponent->generateMesh();
		m_proceduralMeshComponent->OnComponentHit.AddDynamic(this, &AAsteroid::onHit);
	}
}

void AAsteroid::onHit(class UPrimitiveComponent* _comp, class AActor* _otherActor, class UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
	// check if it's a bullet type
	ASimpleBullet* pBullet = Cast<ASimpleBullet>(_otherActor);
	if (pBullet)
	{
		// find where and destroy the right edge
	}
}