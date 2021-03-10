// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Util/Tag.h"
#include "Player/ShipPawn.h"

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
    UNiagaraFunctionLibrary::SpawnSystemAtLocation(this->GetWorld(), this->FireFx, this->GetActorLocation(), this->GetActorRotation());
}

bool AProjectileBase::OnHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
    if (_otherActor->ActorHasTag(Tags::Matiere))
    {
        return false;
    }

    if (_otherActor->ActorHasTag(Tags::Player))
    {
        AShipPawn* shipPawn{ Cast<AShipPawn>(_otherActor) };
        if (shipPawn != nullptr)
        {
            shipPawn->hit(getLocalTeam(), _otherComp, _hit.Item);
        }
    }

    return true;
}

FString&& AProjectileBase::getLocalTeam() const
{
    for (FName tag : this->Tags)
    {
        FString tagStr = tag.ToString();
        if (tagStr.Contains("Team:"))
        {
            TArray<FString> out;
            tagStr.ParseIntoArray(out, TEXT(":"), true);
            if (out.Num() == 2)
            {
                return std::move(out[1]);
            }
        }
    }

    ensure(false);
    return std::move("");
}