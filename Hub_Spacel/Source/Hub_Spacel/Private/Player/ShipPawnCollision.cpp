// Fill out your copyright notice in the Description page of Project Settings.

#include "ShipPawn.h"
#include "CollisionShape.h"
#include "CollisionQueryParams.h"
#include "Gameplay/DestroyActor.h"
#include "GameMode/FlyingGameMode.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Player/SpacelPlayerState.h"
#include "Player/PlayerShipController.h"
#include "DrawDebugHelpers.h"

void AShipPawn::handSweep()
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    // check if ship overlap something
    TArray<FHitResult> hits;
    FVector location { this->GetActorLocation() };

    FCollisionShape collisionShape { FCollisionShape::MakeBox({350, 300, 150}) };
    static const FCollisionObjectQueryParams coqp{ ECC_WorldStatic };

    auto lb_checkCollision = [&](FCollisionShape const& _collision) -> bool
    {
        hits.Empty();
        //DrawDebugSolidBox(world, location, collisionShape.GetExtent(), FColor::Red);

        // for box we need to have a stard != end
        FVector epsilon {0.001f, 0.001f, 0.001f};
        return world->SweepMultiByObjectType(hits, location, location + epsilon, FQuat::Identity, coqp, _collision);
    };

    // check if we have something nearest this pawn
    if (lb_checkCollision(collisionShape))
    {
        auto lb_checkComponent = [&](UInstancedStaticMeshComponent*& _mesh)
        {
            if (_mesh == nullptr || _mesh->GetInstanceCount() == 0)
            {
                return;
            }

            FVector min{}, max{};
            _mesh->GetLocalBounds(min, max);
            FVector box = (max - min) * this->GetTransform().GetScale3D();
            collisionShape = FCollisionShape::MakeBox(box / 2.0f);
            
            int32 index{ 0 };
            while (index < _mesh->GetInstanceCount())
            {
                FTransform worldTransform{};
                if (_mesh->GetInstanceTransform(index, worldTransform, true))
                {
                    location = worldTransform.GetLocation();
                    if (lb_checkCollision(collisionShape) && itemHits(hits))
                    {
                        // manage item hits
                        _mesh->RemoveInstance(index);
                    }
                    else
                    {
                        ++index;
                    }
                }
            }
        };

        lb_checkComponent(this->ProtectionMeshComponent);
        lb_checkComponent(this->SupportMeshComponent);

    }
}

bool AShipPawn::itemHits(TArray<FHitResult> const& _hits)
{
    bool bret = false;
    for (FHitResult const& hit : _hits)
    {
        ADestroyActor* act = Cast<ADestroyActor>(hit.GetActor());
        if (act && !act->IsPendingKill())
        {
            act->dmg(hit);
            bret = true;
        }
    }
    return bret;
}

void AShipPawn::OnComponentHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
    //UE_LOG(LogTemp, Warning, TEXT("Hit"));

    APlayerShipController* playerController = this->GetController<APlayerShipController>();
    if (!ensure(playerController != nullptr)) return;

    playerController->Restart();
    this->UnPossessed();
    this->Destroy();
}