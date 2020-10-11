// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipPawnMovement.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"

void UShipPawnMovement::SetVelocityInLocalSpace(FVector const& _velocity)
{
    InputVelocity = _velocity;
}

void UShipPawnMovement::TickComponent(float _deltaTime, enum ELevelTick _tickType, FActorComponentTickFunction *_thisTickFunction)
{
    if (this->ShouldSkipUpdate(_deltaTime))
    {
        return;
    }

    Super::TickComponent(_deltaTime, _tickType, _thisTickFunction);

    if (!this->PawnOwner || !this->UpdatedComponent)
    {
        return;
    }

    AController const* controller = this->PawnOwner->GetController();

    //if (controller && controller->IsLocalController()) -> if we ignore that we can have fake replication
    {
        FVector deltaLocation = this->InputVelocity * _deltaTime;

        bool bloc = deltaLocation.IsNearlyZero(1e-6f);

        if (!bloc)
        {
            FVector const& oldLocation = UpdatedComponent->GetComponentLocation();
            FRotator const& oldRotation = UpdatedComponent->GetComponentRotation();

            FHitResult hit(1.f);
            this->SafeMoveUpdatedComponent(deltaLocation, oldRotation, true, hit);

            if (hit.IsValidBlockingHit())
            {
                this->HandleImpact(hit, _deltaTime, deltaLocation);
                // Try to slide the remaining distance along the surface.
                this->SlideAlongSurface(deltaLocation, 1.f - hit.Time, hit.Normal, hit, true);
            }
        }
    }
}