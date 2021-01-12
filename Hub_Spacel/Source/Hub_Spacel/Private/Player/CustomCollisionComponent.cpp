// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomCollisionComponent.h"
#include "Player/ShipPawn.h"
#include "Player/ModuleComponent.h"
#include "Player/PlayerShipController.h"
#include "Player/SpacelPlayerState.h"
#include "CollisionShape.h"
#include "CollisionQueryParams.h"
#include "Gameplay/DestroyActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "World/MatiereManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UCustomCollisionComponent::UCustomCollisionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UCustomCollisionComponent::BeginPlay()
{
	Super::BeginPlay();

	if (this->GetNetMode() != ENetMode::NM_DedicatedServer) return;

	m_matiereManager = MakeWeakObjectPtr(Cast<AMatiereManager>(UGameplayStatics::GetActorOfClass(this->GetWorld(), AMatiereManager::StaticClass())));
}

// Called every frame
void UCustomCollisionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (this->GetNetMode() != ENetMode::NM_DedicatedServer) return;

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

	FVector const& scale = m_shipPawnOwner.Get()->GetTransform().GetScale3D();

	auto lb_checkEachInstance = [&](UInstancedStaticMeshComponent*& _mesh, TArray<FVector>& _replicated)
	{
		if (_mesh == nullptr || _mesh->GetInstanceCount() == 0) return;

		FVector min{}, max{};
		_mesh->GetLocalBounds(min, max);
		FVector box = (max - min) * scale;
		collisionShape = FCollisionShape::MakeBox(box / 2.0f);

		FTransform worldTransform{};
		int32 index{ 0 };
		while (index < _mesh->GetInstanceCount())
		{
			// order of "if" is important
			if (_mesh->GetInstanceTransform(index, worldTransform, true)
				&& lb_checkCollision(collisionShape, worldTransform.GetLocation())
				&& saveDestroyActor(saveHits, hits))
			{
				FTransform localTransform{};
				_mesh->GetInstanceTransform(index, localTransform, false);

				if (m_matiereManager.IsValid())
				{
					if (ASpacelPlayerState const* spacelPlayerState = m_shipPawnOwner.Get()->GetPlayerState<ASpacelPlayerState>())
					{
						m_matiereManager.Get()->spawnMatiere(worldTransform.GetLocation(), spacelPlayerState->Team);
					}
				}

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
		hitMatiere(hits);

		// check if we consume all hit item
		if(hits.Num() == 0) return;

		lb_checkEachInstance(m_shipPawnOwner.Get()->ModuleComponent->ProtectionMeshComponent, m_shipPawnOwner.Get()->ModuleComponent->RU_ProtectionLocations);
		lb_checkEachInstance(m_shipPawnOwner.Get()->ModuleComponent->SupportMeshComponent, m_shipPawnOwner.Get()->ModuleComponent->RU_SupportLocations);

		// apply hit on hited actor
		destroyActor(saveHits);

		// check red zone
		FVector min{}, max{};
		m_shipPawnOwner.Get()->DriverMeshComponent->GetLocalBounds(min, max);
		FVector box = (max - min) * m_shipPawnOwner.Get()->GetTransform().GetScale3D();
		collisionShape = FCollisionShape::MakeBox(box / 2.0f);

		if (lb_checkCollision(collisionShape, m_shipPawnOwner.Get()->DriverMeshComponent->GetComponentLocation())
			&& saveDestroyActor(saveHits, hits))
		{
			// apply hit on hited actor
			destroyActor(saveHits);

			m_shipPawnOwner.Get()->kill();
		}
	}
}

void UCustomCollisionComponent::hitMatiere(TArray<FHitResult>& _items) const
{
	int addMatiere {};
	_items.RemoveAll([&addMatiere](FHitResult const& _item)
		{
			if (_item.Actor.IsValid() && _item.Actor.Get()->ActorHasTag("Matiere"))
			{
				if (AMatiereManager* matiere = Cast<AMatiereManager>(_item.Actor.Get()))
				{
					addMatiere += matiere->hit(_item);
					return true;
				}
			}
			return false;
		});

	// make event for add matiere like updateMatiere for decrease or increase matiere
	if (m_shipPawnOwner.IsValid() && addMatiere != int{})
	{
		m_shipPawnOwner.Get()->OnUpdateMatiereDelegate.Broadcast(addMatiere);
	}
}

void UCustomCollisionComponent::destroyActor(TArray<FHitResult>& _items) const
{
	for (FHitResult const& hit : _items)
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

	for (FHitResult const& hit : _items)
	{
		AActor* act = hit.GetActor();
		if (act == nullptr || act->IsPendingKill())
		{
			continue;
		}

		if (ADestroyActor* destroyAct = Cast<ADestroyActor>(act))
		{
			destroyAct->applyDmg();
		}
	}

	_items.Empty();
}

bool UCustomCollisionComponent::saveDestroyActor(TArray<FHitResult>& _items, TArray<FHitResult> const& _hits) const
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