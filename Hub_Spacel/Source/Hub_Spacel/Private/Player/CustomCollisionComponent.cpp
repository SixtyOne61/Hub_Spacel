// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomCollisionComponent.h"
#include "Player/ShipPawn.h"
#include "Player/ModuleComponent.h"
#include "CollisionShape.h"
#include "CollisionQueryParams.h"
#include "Gameplay/DestroyActor.h"
#include "Components/InstancedStaticMeshComponent.h"

// Sets default values for this component's properties
UCustomCollisionComponent::UCustomCollisionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called every frame
void UCustomCollisionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!m_shipPawnOwner.IsValid() && !initShipPawnOwner()) return;

	UWorld* world{ this->GetWorld() };
	if (!ensure(world != nullptr)) return;

	// check if ship overlap something
	FVector const& ownerLocation { m_shipPawnOwner.Get()->GetActorLocation() };

	FCollisionShape collisionShape { FCollisionShape::MakeBox({350, 300, 150}) };
	static const FCollisionObjectQueryParams coqp { FCollisionObjectQueryParams::AllObjects };

	// contain all item hit each sweep
	TArray<FHitResult> hits;
	// contain all item hit by all instance
	TArray<FHitResult> saveHits{};

	auto lb_checkCollision = [&world, &hits](FCollisionShape const& _collision, FVector const& _location) -> bool
	{
		hits.Empty();
		//DrawDebugSolidBox(world, location, collisionShape.GetExtent(), FColor::Red);

		// for box we need to have a stard != end
		FVector epsilon{ 0.001f, 0.001f, 0.001f };
		return world->SweepMultiByObjectType(hits, _location, _location + epsilon, FQuat::Identity, coqp, _collision);
	};

	auto lb_checkEachInstance = [&](UInstancedStaticMeshComponent*& _mesh, TArray<FVector>& _replicated)
	{
		if (_mesh == nullptr || _mesh->GetInstanceCount() == 0) return;

		FVector min{}, max{};
		_mesh->GetLocalBounds(min, max);
		FVector box = (max - min) * m_shipPawnOwner.Get()->GetTransform().GetScale3D();
		collisionShape = FCollisionShape::MakeBox(box / 2.0f);

		FTransform worldTransform{};
		int32 index{ 0 };
		while (index < _mesh->GetInstanceCount())
		{
			// order is important
			if (_mesh->GetInstanceTransform(index, worldTransform, true)
				&& lb_checkCollision(collisionShape, worldTransform.GetLocation())
				&& saveDestroyActor(saveHits, hits))
			{
				FTransform localTransform{};
				_mesh->GetInstanceTransform(index, localTransform, false);

				// manage item hits
				_mesh->RemoveInstance(index);
				// TO DO Check if it's better to make this in temp array
				// for make only one batch for replication
				_replicated.Remove(localTransform.GetLocation());
				
				// go to next item with the same index
				continue;
			}
			++index;
		}
	};

	if (m_shipPawnOwner.Get()->ModuleComponent
		&& lb_checkCollision(collisionShape, ownerLocation))
	{
		lb_checkEachInstance(m_shipPawnOwner.Get()->ModuleComponent->ProtectionMeshComponent, m_shipPawnOwner.Get()->ModuleComponent->RU_ProtectionLocations);
		lb_checkEachInstance(m_shipPawnOwner.Get()->ModuleComponent->SupportMeshComponent, m_shipPawnOwner.Get()->ModuleComponent->RU_SupportLocations);

		// apply hit on hited actor
		for (FHitResult const& hit : saveHits)
		{
			AActor* act = hit.GetActor();
			if (act == nullptr || act->IsPendingKill())
			{
				continue;
			}

			if (ADestroyActor* destroyAct = Cast<ADestroyActor>(act))
			{
				destroyAct->dmg(hit);
			}
		}
	}
}

bool UCustomCollisionComponent::saveDestroyActor(TArray<FHitResult>& _items, TArray<FHitResult> const& _hits)
{
	int32 count { _items.Num() };

	for (FHitResult const& hit : _hits)
	{
		AActor* act { hit.GetActor() };
		if (act == nullptr || act->IsPendingKill())
		{
			continue;
		}

		if (act->ActorHasTag("DestroyActor"))
		{
			_items.Add(hit);
		}
	}

	return count != _items.Num();
}