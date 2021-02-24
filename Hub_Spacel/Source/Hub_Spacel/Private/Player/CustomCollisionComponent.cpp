// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomCollisionComponent.h"
#include "Player/ShipPawn.h"
#include "Player/ModuleComponent.h"
#include "Player/SpacelPlayerState.h"
#include "CollisionQueryParams.h"
#include "Gameplay/DestroyActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "World/MatiereManager.h"
#include "Kismet/GameplayStatics.h"
#include "Util/Tag.h"
#include "DrawDebugHelpers.h"
#include "GameState/SpacelGameState.h"

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

	m_matiereManager = MakeWeakObjectPtr(Cast<AMatiereManager>(UGameplayStatics::GetActorOfClass(this->GetWorld(), AMatiereManager::StaticClass())));
}

bool UCustomCollisionComponent::sweepByProfile(TArray<FHitResult>& _outHit, FVector const& _worldLocation, FName const& _profile, FCollisionShape const& _shape, bool _drawDebug /*= false*/) const
{
	UWorld* world{ this->GetWorld() };
	if (!ensure(world != nullptr)) return false;

	_outHit.Empty();
	if (_drawDebug)
	{
		DrawDebugSolidBox(world, _worldLocation, _shape.GetExtent(), FColor::Red);
	}

	// for box we need to have a stard != end
	static const FVector epsilon{ 0.001f, 0.001f, 0.001f };
	return world->SweepMultiByProfile(_outHit, _worldLocation, _worldLocation + epsilon, FQuat::Identity, _profile, _shape);
}

bool UCustomCollisionComponent::sweepByProfile(TArray<FHitResult> & _outHit, FVector const& _worldLocation, FName const& _profile, FCollisionShape const& _shape, TArray<FName> const& _ignoreTags, bool _drawDebug /*= false*/) const
{
	sweepByProfile(_outHit, _worldLocation, _profile, _shape, _drawDebug);
	
	if (_ignoreTags.Num() == 0)
	{
		return _outHit.Num() != 0;
	}

	_outHit.RemoveAll([&_ignoreTags](FHitResult & _hit)
		{
			if (_hit.Actor.Get() != nullptr && !_hit.Actor.Get()->IsPendingKill())
			{
				for (FName const& ignoreTag : _ignoreTags)
				{
					if (_hit.Actor.Get()->ActorHasTag(ignoreTag))
					{
						return true;
					}
				}

				return false;
			}

			return true;
		});

	return _outHit.Num() != 0;
}

void UCustomCollisionComponent::dispatch(TArray<FHitResult> const& _items) const
{
	struct SInfo
	{
		TWeakObjectPtr<AActor> m_actor { };
		TArray<int32> m_instance { };
	};

	TMap<uint32, SInfo> uniqueActors {};
	for (FHitResult const& item : _items)
	{
		if(item.Actor.Get() == nullptr || item.Actor.Get()->IsPendingKill()) continue;

		uint32 uniqueId = item.Actor.Get()->GetUniqueID();
		if (!uniqueActors.Contains(uniqueId))
		{
			uniqueActors.Add(uniqueId);
			uniqueActors[uniqueId].m_actor = item.Actor;
		}

		uniqueActors[uniqueId].m_instance.Add(item.Item);
	}

	for (TTuple<uint32, SInfo>& uniqueActor : uniqueActors)
	{
		if (ADestroyActor* destroyActor = Cast<ADestroyActor>(uniqueActor.Value.m_actor.Get()))
		{
			destroyActor->applyHit(uniqueActor.Value.m_instance);
		}
	}
}

bool UCustomCollisionComponent::sweepForInstancedStaticMesh(UInstancedStaticMeshComponent*& _mesh, TArray<FVector>& _replicated, TArray<FVector>& _removeReplicated, FVector const& _scale, FName const& _profile, FName const& _teamTag)
{
	if (_mesh == nullptr || _mesh->GetInstanceCount() == 0) return false;

	// for return
	int32 count = _mesh->GetInstanceCount();

	FCollisionShape shape = createCollisionShapeWithLocalBounds<UInstancedStaticMeshComponent>(_mesh, _scale);

	FTransform localTransform{}, worldTransform{};
	int32 index {};
	while (index < _mesh->GetInstanceCount())
	{
		TArray<FHitResult> hits;
		if (_mesh->GetInstanceTransform(index, localTransform, false)
			&& _mesh->GetInstanceTransform(index, worldTransform, true)
			&& sweepByProfile(hits, worldTransform.GetLocation(), _profile, shape, {Tags::Matiere, Tags::Fog, _teamTag }))
		{
			// spawn matiere
			if (m_matiereManager.IsValid())
			{
				if (ASpacelPlayerState const* spacelPlayerState = m_shipPawnOwner.Get()->GetPlayerState<ASpacelPlayerState>())
				{
					m_matiereManager.Get()->spawnMatiere(worldTransform.GetLocation(), spacelPlayerState->Team);
				}
			}

			// remove instance
			_mesh->RemoveInstance(index);
			FVector const& location = localTransform.GetLocation();
			_replicated.Remove(location);
			_removeReplicated.Add(location);

			addScore(hits, EScoreType::Hit);

			// clean actor hit
			dispatch(hits);

			continue;
		}

		++index;
	}

	return count != _mesh->GetInstanceCount();
}

// Called every frame
void UCustomCollisionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!m_shipPawnOwner.IsValid() && !initShipPawnOwner()) return;
	if (m_shipPawnOwner.Get()->DriverMeshComponent == nullptr) return;
	if (m_shipPawnOwner.Get()->m_isKilled) return;

	FName const& profileCollision = m_shipPawnOwner.Get()->DriverMeshComponent->GetCollisionProfileName();

	UWorld* world{ this->GetWorld() };
	if (!ensure(world != nullptr)) return;

	/* check if ship hit something */

	FVector const& ownerLocation { m_shipPawnOwner.Get()->GetActorLocation() };

	TArray<FHitResult> hits;
	if (sweepByProfile(hits, ownerLocation, profileCollision, { FCollisionShape::MakeBox({400, 350, 200}) }))
	{
		// add matiere if we hit it
		hitMatiere(hits);

		// if we hit only matiere return
		if (hits.Num() == 0)
		{
			return;
		}

		FVector const& scale = m_shipPawnOwner.Get()->GetTransform().GetScale3D();
		FString tagTeam = "Team:" + m_shipPawnOwner.Get()->Team.ToString();

		// first check red zone
		FCollisionShape redZoneShape = createCollisionShapeWithLocalBounds<UStaticMeshComponent>(m_shipPawnOwner.Get()->DriverMeshComponent, scale);
		FVector const& redZoneLocation = m_shipPawnOwner.Get()->DriverMeshComponent->GetComponentLocation();
		if (sweepByProfile(hits, redZoneLocation, profileCollision, redZoneShape, { Tags::Matiere, Tags::Fog, *tagTeam }))
		{
			dispatch(hits);
			m_shipPawnOwner.Get()->kill();
			addScore(hits, EScoreType::Kill);
			return; // break flow
		}

		FName prot = m_shipPawnOwner.Get()->ModuleComponent->ProtectionMeshComponent->GetCollisionProfileName();
		// for each module, we need to check each instance
		if (sweepForInstancedStaticMesh(m_shipPawnOwner.Get()->ModuleComponent->ProtectionMeshComponent, m_shipPawnOwner.Get()->ModuleComponent->RU_ProtectionLocations, m_shipPawnOwner.Get()->ModuleComponent->R_RemovedProtectionLocations, scale, profileCollision, *tagTeam))
		{
			m_shipPawnOwner.Get()->OnHitProtectionDelegate.Broadcast();
			m_shipPawnOwner.Get()->RPCClientPlayCameraShake();
		}
		if (sweepForInstancedStaticMesh(m_shipPawnOwner.Get()->ModuleComponent->SupportMeshComponent, m_shipPawnOwner.Get()->ModuleComponent->RU_SupportLocations, m_shipPawnOwner.Get()->ModuleComponent->R_RemovedSupportLocations, scale, profileCollision, *tagTeam))
		{
			m_shipPawnOwner.Get()->OnHitSupportDelegate.Broadcast();
			m_shipPawnOwner.Get()->RPCClientPlayCameraShake();
		}
	}
}

void UCustomCollisionComponent::hitMatiere(TArray<FHitResult>& _items) const
{
	int addMatiere {};
	ASpacelPlayerState const* spacelPlayerState { m_shipPawnOwner.Get()->GetPlayerState<ASpacelPlayerState>() };
	if (spacelPlayerState == nullptr) return;

	FString const& team = spacelPlayerState->Team;
	_items.RemoveAll([&addMatiere, &team](FHitResult const& _item)
		{
			if (_item.Actor.IsValid() && _item.Actor.Get()->ActorHasTag(Tags::Matiere))
			{
				if (AMatiereManager* matiere = Cast<AMatiereManager>(_item.Actor.Get()))
				{
					addMatiere += matiere->hit(_item, team);
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

void UCustomCollisionComponent::hit(FString const& _team, class UPrimitiveComponent* _comp, int32 _index)
{
	if(m_shipPawnOwner.Get() == nullptr) return;
	if(m_shipPawnOwner.Get()->ModuleComponent == nullptr) return;
	if(m_shipPawnOwner.Get()->ModuleComponent->ProtectionMeshComponent == nullptr) return;
	if(m_shipPawnOwner.Get()->ModuleComponent->SupportMeshComponent == nullptr) return;
	if(m_shipPawnOwner.Get()->DriverMeshComponent == nullptr) return;

	uint32 uniqueId { _comp->GetUniqueID() };

	auto lb_removeInstance = [&](UInstancedStaticMeshComponent*& _mesh, TArray<FVector>& _replicated, TArray<FVector>& _removeReplicated)
	{
		if (_mesh == nullptr || _mesh->GetInstanceCount() == 0) return;

		FTransform localTransform{};
		FTransform worldTransform{};

		bool ret = _mesh->GetInstanceTransform(_index, localTransform, false);
		ret &= _mesh->GetInstanceTransform(_index, worldTransform, true);

		if (ret)
		{
			if (m_matiereManager.IsValid())
			{
				if (ASpacelPlayerState const* spacelPlayerState = m_shipPawnOwner.Get()->GetPlayerState<ASpacelPlayerState>())
				{
					m_matiereManager.Get()->spawnMatiere(worldTransform.GetLocation(), spacelPlayerState->Team);
				}
			}

			// manage item hits
			_mesh->RemoveInstance(_index);
			// TO DO Check if it's better to make this in temp array
			// for make only one batch for replication
			FVector const& location = localTransform.GetLocation();
			_replicated.Remove(location);
			_removeReplicated.Add(location);
		}
	};

	auto lb_addScore = [&](EScoreType _type)
	{
		if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())))
		{
			if (*_team != m_shipPawnOwner.Get()->Team)
			{
				spacelGameState->AddScore(_team, _type);
			}
		}
	};

	if (uniqueId == m_shipPawnOwner.Get()->ModuleComponent->ProtectionMeshComponent->GetUniqueID())
	{
		lb_removeInstance(m_shipPawnOwner.Get()->ModuleComponent->ProtectionMeshComponent, m_shipPawnOwner.Get()->ModuleComponent->RU_ProtectionLocations, m_shipPawnOwner.Get()->ModuleComponent->R_RemovedProtectionLocations);
		lb_addScore(EScoreType::Hit);
	}
	else if (uniqueId == m_shipPawnOwner.Get()->ModuleComponent->SupportMeshComponent->GetUniqueID())
	{
		lb_removeInstance(m_shipPawnOwner.Get()->ModuleComponent->SupportMeshComponent, m_shipPawnOwner.Get()->ModuleComponent->RU_SupportLocations, m_shipPawnOwner.Get()->ModuleComponent->R_RemovedSupportLocations);
		lb_addScore(EScoreType::Hit);
	}
	else if (uniqueId == m_shipPawnOwner.Get()->DriverMeshComponent->GetUniqueID())
	{
		m_shipPawnOwner.Get()->kill();
		lb_addScore(EScoreType::Kill);
	}
}

void UCustomCollisionComponent::addScore(TArray<FHitResult> const& _hits, EScoreType _type) const
{
	if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())))
	{
		TSet<FString> teams;
		for (FHitResult const& hit : _hits)
		{
			if (hit.GetActor() != nullptr)
			{
				for (FName const& tag : hit.GetActor()->Tags)
				{
					FString stag = tag.ToString();
					if (stag.Contains("Team:"))
					{
						TArray<FString> out;
						stag.ParseIntoArray(out, TEXT(":"), true);
						if (out.Num() == 2)
						{
							if (*out[1] != m_shipPawnOwner.Get()->Team)
							{
								teams.Add(out[1]);
							}
						}
					}
				}
			}
		}

		for (FString const& team : teams)
		{
			spacelGameState->AddScore(team, _type);
		}
		
	}
}