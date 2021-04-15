// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomCollisionComponent.h"
#include "Player/ShipPawn.h"
#include "Player/ModuleComponent.h"
#include "Player/SpacelPlayerState.h"
#include "CollisionQueryParams.h"
#include "Gameplay/DestroyActor.h"
#include "Gameplay/Bullet/ProjectileBase.h"
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

	world->SweepMultiByProfile(_outHit, _worldLocation, _worldLocation + epsilon, FQuat::Identity, _profile, _shape);
	return _outHit.Num() != 0;
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
			&& sweepByProfile(hits, worldTransform.GetLocation(), _profile, shape, {Tags::Matiere, Tags::Fog, _teamTag, Tags::WorldManager }))
		{
			if (get<AShipPawn>()!= nullptr && get<AShipPawn>()->canTank(hits.Num()))
			{
				// clean actor hit
				dispatch(hits);

				++index;
				continue;
			}

			// spawn matiere
			if (m_matiereManager.IsValid())
			{
				bool needSpawnMatiere = true;
				if (!get()->hasEffect(EEffect::Emp))
				{
					needSpawnMatiere = false;
					// check if we have team player in hits
					for (FHitResult const& hit : hits)
					{
						if (hit.Actor.IsValid())
						{
							for (FName const& name : hit.Actor->Tags)
							{
								if (name.ToString().Contains("Team"))
								{
									needSpawnMatiere = true;
									break;
								}
							}
						}

						if (needSpawnMatiere) break;
					}
				}

				if (needSpawnMatiere)
				{
					if (ASpacelPlayerState const* spacelPlayerState = get()->GetPlayerState<ASpacelPlayerState>())
					{
						m_matiereManager.Get()->spawnMatiere(worldTransform.GetLocation(), spacelPlayerState->R_Team);
					}
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

	if (get() == nullptr && !initShipPawnOwner()) return;
	if (get()->DriverMeshComponent == nullptr) return;
	if (get()->hasEffect(EEffect::Killed)) return;

	FName const& profileCollision = get()->DriverMeshComponent->GetCollisionProfileName();

	UWorld* world{ this->GetWorld() };
	if (!ensure(world != nullptr)) return;

	/* check if ship hit something */

	FVector const& ownerLocation { get()->GetActorLocation() };

	// add matiere if we hit it
	hitMatiere(ownerLocation, profileCollision);

	TArray<FHitResult> hits;
	if (sweepByProfile(hits, ownerLocation, profileCollision, { FCollisionShape::MakeBox({400, 350, 200}) }))
	{
		FVector const& scale = get()->GetTransform().GetScale3D();
		if(get<AShipPawn>() == nullptr) return;
		FString tagTeam = "Team:" + get()->Team.ToString();

		FName prot = get()->ModuleComponent->ProtectionMeshComponent->GetCollisionProfileName();
		// for each module, we need to check each instance
		if (sweepForInstancedStaticMesh(get()->ModuleComponent->ProtectionMeshComponent, get()->ModuleComponent->RU_ProtectionLocations, get()->ModuleComponent->R_RemovedProtectionLocations, scale, profileCollision, *tagTeam))
		{
			get<AShipPawn>()->OnHitProtectionDelegate.Broadcast();
			get<AShipPawn>()->RPCClientPlayCameraShake();
		}
		if (sweepForInstancedStaticMesh(get()->ModuleComponent->SupportMeshComponent, get()->ModuleComponent->RU_SupportLocations, get()->ModuleComponent->R_RemovedSupportLocations, scale, profileCollision, *tagTeam))
		{
			get<AShipPawn>()->OnHitSupportDelegate.Broadcast();
			get<AShipPawn>()->RPCClientPlayCameraShake();
		}

		// end check red zone
		FCollisionShape redZoneShape = createCollisionShapeWithLocalBounds<UStaticMeshComponent>(get()->DriverMeshComponent, scale);
		FVector const& redZoneLocation = get()->DriverMeshComponent->GetComponentLocation();
		if (sweepByProfile(hits, redZoneLocation, profileCollision, redZoneShape, { Tags::Matiere, Tags::Fog, *tagTeam, Tags::WorldManager }))
		{
			dispatch(hits);
			if (!get<AShipPawn>()->canTank(hits.Num()))
			{
				get<AShipPawn>()->kill();
				addScore(hits, EScoreType::Kill);
				return; // break flow
			}
		}
	}
}

void UCustomCollisionComponent::hitMatiere(FVector const& _ownerLocation, FName const& _profileCollision) const
{
	TArray<FHitResult> hits;
	if (sweepByProfile(hits, _ownerLocation, _profileCollision, { FCollisionShape::MakeBox({800, 800, 800}) }))
	{
		int addMatiere{};
		ASpacelPlayerState const* spacelPlayerState = get()->GetPlayerState<ASpacelPlayerState>();
		if (spacelPlayerState == nullptr) return;

		FString const& team = spacelPlayerState->R_Team;
		hits.RemoveAll([&addMatiere, &team](FHitResult const& _item)
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
		if (get<AShipPawn>() != nullptr && addMatiere != int{})
		{
			get<AShipPawn>()->OnUpdateMatiereDelegate.Broadcast(addMatiere);
		}
	}
}

void UCustomCollisionComponent::hit(FString const& _team, int32 _playerId, class UPrimitiveComponent* _comp, int32 _index)
{
	if(get() == nullptr) return;
	if(get()->ModuleComponent == nullptr) return;
	if(get()->ModuleComponent->ProtectionMeshComponent == nullptr) return;
	if(get()->ModuleComponent->SupportMeshComponent == nullptr) return;
	if(get()->DriverMeshComponent == nullptr) return;

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
				if (ASpacelPlayerState const* spacelPlayerState = get()->GetPlayerState<ASpacelPlayerState>())
				{
					m_matiereManager.Get()->spawnMatiere(worldTransform.GetLocation(), spacelPlayerState->R_Team);
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
			if (*_team != get()->Team)
			{
				spacelGameState->AddScore(_team, _playerId, _type);
			}
		}
	};

	if (uniqueId == get()->ModuleComponent->ProtectionMeshComponent->GetUniqueID())
	{
		if (get<AShipPawn>() != nullptr)
		{
			if (!get<AShipPawn>()->canTank(1))
			{
				lb_removeInstance(get()->ModuleComponent->ProtectionMeshComponent, get()->ModuleComponent->RU_ProtectionLocations, get()->ModuleComponent->R_RemovedProtectionLocations);
				lb_addScore(EScoreType::Hit);
			}
		}
	}
	else if (uniqueId == get()->ModuleComponent->SupportMeshComponent->GetUniqueID())
	{
		if (get<AShipPawn>() != nullptr)
		{
			if (!get<AShipPawn>()->canTank(1))
			{
				lb_removeInstance(get()->ModuleComponent->SupportMeshComponent, get()->ModuleComponent->RU_SupportLocations, get()->ModuleComponent->R_RemovedSupportLocations);
				lb_addScore(EScoreType::Hit);
			}
		}
	}
	else if (uniqueId == get()->DriverMeshComponent->GetUniqueID())
	{
		if (get<AShipPawn>() != nullptr)
		{
			if (!get<AShipPawn>()->canTank(1))
			{
				get<AShipPawn>()->kill();
				lb_addScore(EScoreType::Kill);
			}
		}
	}
}

void UCustomCollisionComponent::addScore(TArray<FHitResult> const& _hits, EScoreType _type) const
{
	if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())))
	{
		if (get<AShipPawn>() != nullptr && get<AShipPawn>()->hasEffect(EEffect::Emp))
		{
			spacelGameState->AddScore(get<AShipPawn>()->m_lastTeamEmp.ToString(), get<AShipPawn>()->m_lastPlayerIdEmp, EScoreType::Emp);
		}
		else
		{
			TSet<TTuple<int32, FString>> playerInfos;
			for (FHitResult const& hit : _hits)
			{
				if (hit.GetActor() != nullptr)
				{
					TTuple<int32, FString> playerInfo;
					if (AProjectileBase* projectileBase = Cast<AProjectileBase>(hit.GetActor()))
					{
						playerInfo.Key = projectileBase->R_PlayerIdOwner;
					}

					for (FName const& tag : hit.GetActor()->Tags)
					{
						FString stag = tag.ToString();
						if (stag.Contains("Team:"))
						{
							TArray<FString> out;
							stag.ParseIntoArray(out, TEXT(":"), true);
							if (out.Num() == 2)
							{
								if (*out[1] != get()->Team)
								{
									playerInfo.Value = out[1];
								}
							}
						}
					}

					playerInfos.Add(playerInfo);
				}
			}

			for (TTuple<int32, FString> const& playerInfo : playerInfos)
			{
				spacelGameState->AddScore(playerInfo.Value, playerInfo.Key, _type);
			}
		}
	}
}