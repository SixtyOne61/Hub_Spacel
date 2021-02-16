// Fill out your copyright notice in the Description page of Project Settings.


#include "LaserBullet.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraFunctionLibrary.h"
#include "Util/Tag.h"
#include "Player/ShipPawn.h"
#include "Player/ModuleComponent.h"

// Sets default values
ALaserBullet::ALaserBullet()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    ProjectileCollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("ProjectileCollision"));
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

    if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        if (!ensure(ProjectileCollisionComponent != nullptr)) return;
        this->ProjectileCollisionComponent->OnComponentHit.AddDynamic(this, &ALaserBullet::OnComponentHit);
    }

    // spawn fx fire
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(this->GetWorld(), this->FireFx, this->GetActorLocation(), this->GetActorRotation());

    FTimerHandle handle;
    this->GetWorldTimerManager().SetTimer(handle, this, &ALaserBullet::InitProfileCollision, 1.5f, false, 0.0f);
}

void ALaserBullet::InitProfileCollision()
{
    if (this->ProjectileCollisionComponent != nullptr)
    {
        this->ProjectileCollisionComponent->SetCollisionProfileName("Projectil");
    }
    else
    {
        FTimerHandle handle;
        this->GetWorldTimerManager().SetTimer(handle, this, &ALaserBullet::InitProfileCollision, 1.5f, false, 0.0f);
    }
}

void ALaserBullet::applyHit(TArray<int32>& _instance)
{
    Super::applyHit(_instance);
    this->Destroy();
}

void ALaserBullet::OnComponentHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
    if (_otherActor->ActorHasTag(Tags::Matiere))
    {
        return;
    }

    if (_otherActor->ActorHasTag(Tags::Player))
    {
        AShipPawn* shipPawn{ Cast<AShipPawn>(_otherActor) };
        if (shipPawn != nullptr)
        {
            shipPawn->hit(_otherComp, _hit.Item);
        }
    }
    this->Destroy();
}
