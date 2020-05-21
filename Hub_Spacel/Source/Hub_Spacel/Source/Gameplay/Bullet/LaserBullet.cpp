// Fill out your copyright notice in the Description page of Project Settings.


#include "LaserBullet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"

// Sets default values
ALaserBullet::ALaserBullet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));

    ProjectileCollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("ProjectileCollision"));
    ProjectileCollisionComponent->SetCollisionProfileName(CollisionProfileName);
    RootComponent = ProjectileCollisionComponent;

    LaserMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaserMesh"));
    LaserMeshComponent->SetupAttachment(ProjectileCollisionComponent);
}

// Called when the game starts or when spawned
void ALaserBullet::BeginPlay()
{
	Super::BeginPlay();
	
    setupMaterial();
}

// Called every frame
void ALaserBullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALaserBullet::setupMaterial()
{
    // setup material
    if (!MatBullet)
    {
        return;
    }

    if (UMaterialInstanceDynamic * customMat = UMaterialInstanceDynamic::Create(MatBullet, LaserMeshComponent))
    {
        LaserMeshComponent->CustomDepthStencilValue = 3;
        LaserMeshComponent->bRenderCustomDepth = true;
        LaserMeshComponent->SetMaterial(0, customMat);
    }
}

