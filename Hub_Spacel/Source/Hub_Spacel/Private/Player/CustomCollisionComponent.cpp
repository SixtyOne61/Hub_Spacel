// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomCollisionComponent.h"
#include "Player/ShipPawn.h"
#include "Player/ModuleComponent.h"
#include "Player/SpacelPlayerState.h"
#include "CollisionQueryParams.h"
#include "Gameplay/DestroyActor.h"
#include "Gameplay/Bullet/ProjectileBase.h"
#include "Gameplay/Mission/MissionActor.h"
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

bool UCustomCollisionComponent::sweepForInstancedStaticMesh(UInstancedStaticMeshComponent*& _mesh, TArray<FVector_NetQuantize>& _replicated, TArray<FVector_NetQuantize>& _removeReplicated, FVector const& _scale, FName const& _profile, FName const& _teamTag)
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
			AShipPawn * shipPawn = get<AShipPawn>();
			if (shipPawn != nullptr && shipPawn->canTank(hits.Num()))
			{
				// clean actor hit
				dispatch(hits);

				++index;
				continue;
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

	ACommonPawn* commonPawn = get();
	if (commonPawn == nullptr && !initShipPawnOwner()) return;
	if (commonPawn->DriverMeshComponent == nullptr) return;
	if (commonPawn->hasEffect(EEffect::Killed)) return;

	FName const& profileCollision = commonPawn->DriverMeshComponent->GetCollisionProfileName();

	UWorld* world{ this->GetWorld() };
	if (!ensure(world != nullptr)) return;

	// check if ship hit something

	FVector const& ownerLocation { commonPawn->GetActorLocation() };

	// add matiere if we hit it
	hitMatiere(ownerLocation, profileCollision);
	// resolve mission
	hitMission(ownerLocation, profileCollision);

	TArray<FHitResult> hits;
	if (sweepByProfile(hits, ownerLocation, profileCollision, { FCollisionShape::MakeBox({400, 350, 200}) }))
	{
		FVector const& scale = commonPawn->GetTransform().GetScale3D();
		if (AShipPawn* pawn = get<AShipPawn>())
		{
			FString tagTeam = "Team:" + pawn->Team.ToString();

			if (UModuleComponent* moduleComponent = pawn->ModuleComponent)
			{
				FName prot = moduleComponent->ProtectionMeshComponent->GetCollisionProfileName();
				// for each module, we need to check each instance
				if (sweepForInstancedStaticMesh(moduleComponent->ProtectionMeshComponent, moduleComponent->RU_ProtectionLocations, moduleComponent->RemovedProtectionLocations, scale, profileCollision, *tagTeam))
				{
					pawn->RPCClientPlayCameraShake();
				}
				if (sweepForInstancedStaticMesh(moduleComponent->SupportMeshComponent, moduleComponent->RU_SupportLocations, moduleComponent->RemovedSupportLocations, scale, profileCollision, *tagTeam))
				{
					pawn->RPCClientPlayCameraShake();
				}

				// end check red zone
				FCollisionShape redZoneShape = createCollisionShapeWithLocalBounds<UStaticMeshComponent>(get()->DriverMeshComponent, scale);
				FVector const& redZoneLocation = pawn->DriverMeshComponent->GetComponentLocation();
				if (sweepByProfile(hits, redZoneLocation, profileCollision, redZoneShape, { Tags::Matiere, Tags::Fog, *tagTeam, Tags::WorldManager }))
				{
					if (!pawn->canTank(hits.Num()))
					{
						bool first = true;
						// find team, if exist, of killer then broadcast event
						for (auto const& hit : hits)
						{
							if (AActor* actor = hit.GetActor())
							{
								if (!actor->IsPendingKill())
								{
									if (first) // only one player mark point
									{
										addScore({hit}, EScoreType::Kill);
										first = false;
									}

									for (FName const& tag : actor->Tags)
									{
										FString stag = tag.ToString();
										if (stag.Contains("Team:"))
										{
											TArray<FString> out;
											stag.ParseIntoArray(out, TEXT(":"), true);
											if (out.Num() == 2)
											{
												pawn->OnKill.broadcast(pawn->Team.ToString(), *out[1]);
											}
										}
									}
								}
							}
						}

						// spawn matiere
						if (m_matiereManager.IsValid())
						{
							// check if we have team player in hits
							for (FHitResult const& hit : hits)
							{
								if (hit.Actor.IsValid())
								{
									for (FName const& name : hit.Actor->Tags)
									{
										if (name.ToString().Contains("Team"))
										{
											if (ASpacelPlayerState const* spacelPlayerState = get()->GetPlayerState<ASpacelPlayerState>())
											{
												m_matiereManager.Get()->spawnMatiere(get()->GetActorLocation(), spacelPlayerState->R_Team);
												return; //break flow
											}
										}
									}
								}
							}
						}

						pawn->kill();
					}

					dispatch(hits);
				}
			}
		}
	}
}

void UCustomCollisionComponent::hitMission(FVector const& _ownerLocation, FName const& _profileCollision) const
{
	// TO DO optim : check if we have a mission
	TArray<FHitResult> hits;
	if (sweepByProfile(hits, _ownerLocation, _profileCollision, { FCollisionShape::MakeBox({600, 600, 600}) }))
	{
		hits.RemoveAll([](FHitResult const& _item)
			{
				if (_item.Actor.IsValid() && _item.Actor.Get()->ActorHasTag(Tags::Mission))
				{
					return false;
				}
				return true;
			});

		ASpacelPlayerState const* spacelPlayerState = get()->GetPlayerState<ASpacelPlayerState>();
		if (spacelPlayerState == nullptr) return;

		FString const& team = spacelPlayerState->R_Team;

		for (auto const& hit : hits)
		{
			if (AMissionActor* mission = Cast<AMissionActor>(hit.Actor))
			{
				mission->hit(get<AShipPawn>(), team);
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
		if (AShipPawn* pawn = get<AShipPawn>())
		{
			if (addMatiere != int{})
			{
				pawn->OnUpdateMatiereDelegate.Broadcast(addMatiere);
			}
		}
	}
}

void UCustomCollisionComponent::hit(FString const& _team, int32 _playerId, class UPrimitiveComponent* _comp, int32 _index, FVector const& _otherLocation)
{
	AShipPawn* shipPawn = get<AShipPawn>();
	if(shipPawn == nullptr) return;
	UModuleComponent* moduleComponent = shipPawn->ModuleComponent;
	if(moduleComponent == nullptr) return;
	if(moduleComponent->ProtectionMeshComponent == nullptr) return;
	if(moduleComponent->SupportMeshComponent == nullptr) return;
	if(shipPawn->DriverMeshComponent == nullptr) return;

	uint32 uniqueId { _comp->GetUniqueID() };

	auto lb_removeInstance = [&](UInstancedStaticMeshComponent*& _mesh, TArray<FVector_NetQuantize>& _replicated, TArray<FVector_NetQuantize>& _removeReplicated)
	{
		if (_mesh == nullptr || _mesh->GetInstanceCount() == 0) return;

		FTransform localTransform{};
		FTransform worldTransform{};

		bool ret = _mesh->GetInstanceTransform(_index, localTransform, false);
		ret &= _mesh->GetInstanceTransform(_index, worldTransform, true);

		if (ret)
		{
			// manage item hits
			_mesh->RemoveInstance(_index);
			FVector_NetQuantize const& location = localTransform.GetLocation();
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

	auto lb_generic = [&](UInstancedStaticMeshComponent*& _mesh, TArray<FVector_NetQuantize>& _replicated, TArray<FVector_NetQuantize>& _removeReplicated)
	{
		if (!shipPawn->canTank(1))
		{
			lb_removeInstance(_mesh, _replicated, _removeReplicated);
			lb_addScore(EScoreType::Hit);

			// for feedback
			shipPawn->RPCClientDamageIndicator(_otherLocation);
		}
	};


	if (uniqueId == get()->ModuleComponent->ProtectionMeshComponent->GetUniqueID())
	{
		lb_generic(moduleComponent->ProtectionMeshComponent,
					moduleComponent->RU_ProtectionLocations,
					moduleComponent->RemovedProtectionLocations);
	}
	else if (uniqueId == get()->ModuleComponent->SupportMeshComponent->GetUniqueID())
	{
		lb_generic(moduleComponent->SupportMeshComponent,
					moduleComponent->RU_SupportLocations,
					moduleComponent->RemovedSupportLocations);
	}
	else if (uniqueId == get()->DriverMeshComponent->GetUniqueID())
	{
		if (!shipPawn->canTank(1) && !shipPawn->hasEffect(EEffect::Killed))
		{
			if (m_matiereManager.IsValid())
			{
				if (ASpacelPlayerState const* spacelPlayerState = get()->GetPlayerState<ASpacelPlayerState>())
				{
					m_matiereManager.Get()->spawnMatiere(shipPawn->DriverMeshComponent->GetComponentLocation(), spacelPlayerState->R_Team);
				}
			}

			lb_addScore(EScoreType::Kill);
			shipPawn->OnKill.broadcast(shipPawn->Team.ToString(), _team);

			shipPawn->kill();
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