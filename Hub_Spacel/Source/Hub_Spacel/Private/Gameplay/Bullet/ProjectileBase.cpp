// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "NiagaraFunctionLibrary.h"
#include "Util/Tag.h"
#include "Player/ShipPawn.h"
#include "Net/UnrealNetwork.h"

AProjectileBase::AProjectileBase()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = false;

    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
}

void AProjectileBase::BeginPlay()
{
    Super::BeginPlay();

    // spawn fx fire
    //UNiagaraFunctionLibrary::SpawnSystemAtLocation(this->GetWorld(), this->FireFx, this->GetActorLocation(), this->GetActorRotation());
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
            shipPawn->hit(getLocalTeam(), R_PlayerIdOwner, _otherComp, _hit.Item);
        }
    }

    if (_otherActor->ActorHasTag(Tags::Asteroide))
    {
        if (GetWorld() != nullptr && GetWorld()->GetGameState() != nullptr)
        {
            TArray<APlayerState*> playerStates = GetWorld()->GetGameState()->PlayerArray;
            for (APlayerState* playerState : playerStates)
            {
                if (playerState != nullptr && playerState->PlayerId == this->R_PlayerIdOwner)
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
    DOREPLIFETIME(AProjectileBase, R_PlayerIdOwner);
}