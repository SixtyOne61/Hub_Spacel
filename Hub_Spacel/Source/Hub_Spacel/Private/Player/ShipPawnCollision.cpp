// Fill out your copyright notice in the Description page of Project Settings.

#include "ShipPawn.h"
#include "CollisionShape.h"
#include "CollisionQueryParams.h"
#include "Gameplay/DestroyActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "DrawDebugHelpers.h"

void AShipPawn::RPCServerHandSweep_Implementation()
{
    // releviant on server side

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
        DrawDebugSolidBox(world, location, collisionShape.GetExtent(), FColor::Red);

        // for box we need to have a stard != end
        FVector epsilon {0.001f, 0.001f, 0.001f};
        return world->SweepMultiByObjectType(hits, location, location + epsilon, FQuat::Identity, coqp, _collision);
    };

    // check if we have something nearest this pawn
    if (lb_checkCollision(collisionShape))
    {
        auto lb_checkComponent = [&](UInstancedStaticMeshComponent*& _mesh, TArray<FVector>& _removedLocation)
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
                FTransform localTransform{}, worldTransform{};
                if (_mesh->GetInstanceTransform(index, worldTransform, true) 
                    && _mesh->GetInstanceTransform(index, localTransform, false))
                {
                    location = worldTransform.GetLocation();
                    if (lb_checkCollision(collisionShape) && itemHits(hits))
                    {
                        // manage item hits
                        _mesh->RemoveInstance(index);
                        _removedLocation.Add(localTransform.GetLocation());
                    }
                    else
                    {
                        ++index;
                    }
                }
            }
        };

        TArray<FVector> protRemovedLocation{}, suppRemovedLocation{};
        lb_checkComponent(this->ProtectionMeshComponent, protRemovedLocation);
        lb_checkComponent(this->SupportMeshComponent, suppRemovedLocation);

        if (protRemovedLocation.Num() != 0 || suppRemovedLocation.Num() != 0)
        {
            // replicated this information on each client
            RPCClientRemoveInstance(protRemovedLocation, suppRemovedLocation);
        }
    }
}

bool AShipPawn::itemHits(TArray<FHitResult> const& _hits)
{
    bool bret = false;
    for (FHitResult const& hit : _hits)
    {
        if (ADestroyActor* act = Cast<ADestroyActor>(hit.GetActor()))
        {
            act->hit(hit);
            bret = true;
        }
    }
    return bret;
}

void AShipPawn::RPCClientRemoveInstance_Implementation(TArray<FVector> const& _protRemovedLocation, TArray<FVector> const& _suppRemovedLocation)
{
    auto lb_remove = [](UInstancedStaticMeshComponent*& _mesh, TArray<FVector> const& _removedLocation)
    {
        if (_mesh == nullptr)
        {
            return;
        }

        for (FVector location : _removedLocation)
        {
            for (int32 i = 0; i < _mesh->GetInstanceCount(); ++i)
            {
                FTransform transform{};
                if (_mesh->GetInstanceTransform(i, transform, false) && transform.GetLocation() == location)
                {
                    _mesh->RemoveInstance(i);
                    break;
                }
            }
        }
    };

    lb_remove(this->ProtectionMeshComponent, _protRemovedLocation);
    lb_remove(this->SupportMeshComponent, _suppRemovedLocation);
}