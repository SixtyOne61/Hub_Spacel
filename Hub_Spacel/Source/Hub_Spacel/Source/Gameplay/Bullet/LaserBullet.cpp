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
    if (!ensure(ProjectileCollisionComponent != nullptr)) return;
    ProjectileCollisionComponent->SetCollisionProfileName(CollisionProfileName);
    ProjectileCollisionComponent->OnComponentHit.AddDynamic(this, &ALaserBullet::OnComponentHit);
    RootComponent = ProjectileCollisionComponent;

    LaserMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LaserMesh"));
    if (!ensure(LaserMeshComponent != nullptr)) return;
    LaserMeshComponent->SetupAttachment(ProjectileCollisionComponent);
}

// Called when the game starts or when spawned
void ALaserBullet::BeginPlay()
{
	Super::BeginPlay();
	
    setupMaterial();

    if (this->HasAuthority())
    {
        this->SetReplicates(true);
        this->SetReplicateMovement(true);
    }
}

// Called every frame
void ALaserBullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ALaserBullet::setupMaterial()
{
    // setup material
    if (!ensure(this->MatBullet != nullptr)) return;
    if (!ensure(this->LaserMeshComponent != nullptr)) return;

    if (UMaterialInstanceDynamic * customMat = UMaterialInstanceDynamic::Create(this->MatBullet, this->LaserMeshComponent))
    {
        this->LaserMeshComponent->CustomDepthStencilValue = 3;
        this->LaserMeshComponent->bRenderCustomDepth = true;
        this->LaserMeshComponent->SetMaterial(0, customMat);
    }
}

void ALaserBullet::OnComponentHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
    this->Destroy();
}
