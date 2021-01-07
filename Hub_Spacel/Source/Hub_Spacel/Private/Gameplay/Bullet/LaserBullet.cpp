// Fill out your copyright notice in the Description page of Project Settings.


#include "LaserBullet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraFunctionLibrary.h"

// Sets default values
ALaserBullet::ALaserBullet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));

    ProjectileCollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("ProjectileCollision"));
    if (!ensure(ProjectileCollisionComponent != nullptr)) return;
    ProjectileCollisionComponent->OnComponentHit.AddDynamic(this, &ALaserBullet::OnComponentHit);
    RootComponent = ProjectileCollisionComponent;
}

// Called when the game starts or when spawned
void ALaserBullet::BeginPlay()
{
	Super::BeginPlay();

    if (this->HasAuthority())
    {
        this->SetReplicates(true);
        this->SetReplicateMovement(true);
    }

    // spawn fx fire
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(this->GetWorld(), this->FireFx, this->GetActorLocation(), this->GetActorRotation());
}

void ALaserBullet::dmg(FHitResult const& _info)
{
    Super::dmg(_info);
    if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        this->Destroy();
    }
}

void ALaserBullet::OnComponentHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
    this->Destroy();
}
