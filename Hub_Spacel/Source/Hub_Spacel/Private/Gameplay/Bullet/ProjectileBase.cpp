// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "GameState/SpacelGameState.h"
#include "Util/Tag.h"
#include "Player/ShipPawn.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

AProjectileBase::AProjectileBase()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

    ProjectileCollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("ProjectileCollision"));
    RootComponent = ProjectileCollisionComponent;
}

void AProjectileBase::BeginPlay()
{
    Super::BeginPlay();

    if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        if (!ensure(ProjectileCollisionComponent != nullptr)) return;
        this->ProjectileCollisionComponent->OnComponentHit.AddDynamic(this, &AProjectileBase::OnComponentHit);
    }
}

void AProjectileBase::applyHit(TArray<int32>& _instance)
{
    Super::applyHit(_instance);
    this->Destroy();
}

void AProjectileBase::OnComponentHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
    if (this->OnHit(_hitComp, _otherActor, _otherComp, _normalImpulse, _hit))
    {
        this->Destroy();
    }
}

bool AProjectileBase::OnHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
    if (_otherActor->ActorHasTag(Tags::Matiere))
    {
        return false;
    }

    if (_otherActor->ActorHasTag(Tags::Player))
    {
        if (AShipPawn* shipPawn = Cast<AShipPawn>(_otherActor))
        {
            shipPawn->hit(getLocalTeam(), PlayerIdOwner, _otherComp, _hit.Item, this->GetActorLocation());
        }
    }

    if (_otherActor->ActorHasTag(Tags::Asteroide))
    {
        if (GetWorld() != nullptr && GetWorld()->GetGameState() != nullptr)
        {
            TArray<APlayerState*> playerStates = GetWorld()->GetGameState()->PlayerArray;
            for (APlayerState* playerState : playerStates)
            {
                if (playerState != nullptr && playerState->PlayerId == this->PlayerIdOwner)
                {
                    if (AShipPawn* pawn = playerState->GetPawn<AShipPawn>())
                    {
                        pawn->farmAsteroide();
                        break;
                    }
                }
            }
        }
    }

    if (_otherActor->ActorHasTag(Tags::Mission))
    {
        if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())))
        {
            spacelGameState->AddScore(getLocalTeam(), PlayerIdOwner, EScoreType::Hit);
        }
    }

    return true;
}

FString AProjectileBase::getLocalTeam() const
{
    return R_Team.ToString();
}

void AProjectileBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AProjectileBase, R_Team);
}