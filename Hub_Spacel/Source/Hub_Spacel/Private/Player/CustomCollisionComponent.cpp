// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomCollisionComponent.h"
#include "Player/ShipPawn.h"
#include "Player/ModuleComponent.h"
#include "Player/SpacelPlayerState.h"
#include "CollisionQueryParams.h"
#include "Gameplay/DestroyActor.h"
#include "Gameplay/Bullet/ProjectileBase.h"
#include "Gameplay/Mission/MissionActor.h"
#include "Mesh/AnimatedSpacelMeshComponent.h"
#include "Mesh/EmergencyInstancedMeshComponent.h"
#include "World/MatiereManager.h"
#include "Kismet/GameplayStatics.h"
#include "Util/Tag.h"
#include "DrawDebugHelpers.h"
#include "GameState/SpacelGameState.h"
#include "Gameplay/Skill/HealPackBullet.h"
#include "Gameplay/Bullet/EmpBullet.h"

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
		TWeakObjectPtr<UPrimitiveComponent> m_component { };
		int32 m_instance;
		FVector m_location;
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
			uniqueActors[uniqueId].m_component = item.Component;
			uniqueActors[uniqueId].m_location = item.Location;
		}

		uniqueActors[uniqueId].m_instance = item.Item;
	}

	for (TTuple<uint32, SInfo>& uniqueActor : uniqueActors)
	{
		if (ADestroyActor* destroyActor = Cast<ADestroyActor>(uniqueActor.Value.m_actor.Get()))
		{
			TArray<int32> locs;
			locs.Add(uniqueActor.Value.m_instance);
			destroyActor->applyHit(locs);
		}
		else if (AShipPawn* otherPawn = Cast<AShipPawn>(uniqueActor.Value.m_actor.Get()))
		{
			if (ACommonPawn* pawn = get())
			{
				if (APlayerState* playerState = pawn->GetPlayerState())
				{
					otherPawn->hit(get()->Team.ToString(), playerState->PlayerId,
								uniqueActor.Value.m_component.Get(),
								uniqueActor.Value.m_instance,
								uniqueActor.Value.m_location, pawn);
				}
			}
		}
	}
}

bool UCustomCollisionComponent::sweepForInstancedStaticMesh(UAnimatedSpacelMeshComponent*& _mesh, FVector const& _scale, FName const& _profile, FName const& _teamTag)
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
			if (shipPawn != nullptr && shipPawn->canTank(hits))
			{
				// clean actor hit
				++index;
			}
			else
			{
				// remove instance
				_mesh->RPCNetMulticastRemove(localTransform.GetLocation());

				addScore(hits, EScoreType::Hit);

			}

			// clean actor hit
			dispatch(hits);
		}
		else
		{
			++index;
		}
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
	// hit heal
	hitHeal(ownerLocation, profileCollision);

	TArray<FHitResult> hits;
	if (sweepByProfile(hits, ownerLocation, profileCollision, { FCollisionShape::MakeBox({400, 350, 200}) }))
	{
		FVector const& scale = commonPawn->GetTransform().GetScale3D();
		if (AShipPawn* pawn = get<AShipPawn>())
		{
			FString tagTeam = "Team:" + pawn->Team.ToString();

			FName prot = pawn->ProtectionComponent->GetCollisionProfileName();
			// for each module, we need to check each instance
			if (sweepForInstancedStaticMesh(pawn->ProtectionComponent, scale, profileCollision, *tagTeam))
			{
				pawn->RPCClientPlayCameraShake(EImpactType::Obstacle);
			}
			if (sweepForInstancedStaticMesh(pawn->SupportComponent, scale, profileCollision, *tagTeam))
			{
				pawn->RPCClientPlayCameraShake(EImpactType::Obstacle);
			}

			UAnimatedSpacelMeshComponent* tmp = Cast<UAnimatedSpacelMeshComponent>(pawn->EmergencyComponent);
			if (sweepForInstancedStaticMesh(tmp, scale, profileCollision, *tagTeam))
			{
				pawn->RPCClientPlayCameraShake(EImpactType::Obstacle);
			}

			// end check red zone
			FVector const& redZoneLocation = pawn->DriverMeshComponent->GetComponentLocation();
			FCollisionShape redZoneShape = createCollisionShapeWithLocalBounds<UStaticMeshComponent>(get()->DriverMeshComponent, scale);
			if (sweepByProfile(hits, redZoneLocation, profileCollision, redZoneShape, { Tags::Matiere, Tags::Fog, *tagTeam, Tags::WorldManager }))
			{
				if (!pawn->canTank(hits))
				{
					killersProcess(hits);

					// spawn matiere
					spawnMatiere();

					// check if we collide a player
					int32 idCollider { -1 };
					for (auto hit : hits)
					{
						if (ACommonPawn* otherPawn = Cast<ACommonPawn>(hit.Actor))
						{
							if (auto otherPlayerState = otherPawn->GetPlayerState())
							{
								idCollider = otherPlayerState->PlayerId;
								break;
							}
						}
					}
					pawn->kill(idCollider);
				}

				dispatch(hits);
			}
		}
	}
}

void UCustomCollisionComponent::killersProcess(TArray<FHitResult> const& _hits)
{
	if (AShipPawn* pawn = get<AShipPawn>())
	{
		bool isScored = false;
		// find team, if exist, of killer then broadcast event
		for (auto const& hit : _hits)
		{
			if (AActor* actor = hit.GetActor())
			{
				if (!actor->IsPendingKill())
				{
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

								if (!isScored)
								{
									addScore({ hit }, EScoreType::Kill);
									isScored = true;
								}
							}
						}
					}
				}
			}
		}
	}
}

void UCustomCollisionComponent::spawnMatiere()
{
	// spawn matiere
	if (m_matiereManager.IsValid())
	{
		if (APawn* pawn = get<APawn>())
		{
			if (ASpacelPlayerState const* spacelPlayerState = pawn->GetPlayerState<ASpacelPlayerState>())
			{
				m_matiereManager.Get()->spawnMatiere(pawn->GetActorLocation(), spacelPlayerState->R_Team);
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
				return !(_item.Actor.IsValid() && _item.Actor.Get()->ActorHasTag(Tags::Mission));
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
				pawn->OnUpdateMatiereDelegate.Broadcast(addMatiere, EMatiereOrigin::Kill);
			}
		}
	}
}

void UCustomCollisionComponent::hitHeal(FVector const& _ownerLocation, FName const& _profileCollision) const
{
	TArray<FHitResult> hits;
	if (sweepByProfile(hits, _ownerLocation, _profileCollision, { FCollisionShape::MakeBox({600, 600, 600}) }))
	{
		hits.RemoveAll([](FHitResult const& _item)
			{
				return !(_item.Actor.IsValid() && _item.Actor.Get()->ActorHasTag(Tags::HealPack));
			});

		ASpacelPlayerState const* spacelPlayerState = get()->GetPlayerState<ASpacelPlayerState>();
		if (spacelPlayerState == nullptr) return;

		FString const& team = spacelPlayerState->R_Team;

		for (auto const& hit : hits)
		{
			if (AHealPackBullet* healPack = Cast<AHealPackBullet>(hit.Actor))
			{
				if (!healPack->IsPendingKill())
				{
					if (healPack->R_Team == *team)
					{
						// heal
						if (AShipPawn* pawn = get<AShipPawn>())
						{
							pawn->heal(healPack->Value);
						}
					}

					healPack->Destroy();
				}
			}
		}
	}
}

void UCustomCollisionComponent::hitEmp(AActor* _otherActor) const
{
	if (AEmpBullet* empBullet = Cast<AEmpBullet>(_otherActor))
	{
		if (!empBullet->IsPendingKill())
		{
			ASpacelPlayerState const* spacelPlayerState = get()->GetPlayerState<ASpacelPlayerState>();
			if (spacelPlayerState == nullptr) return;

			FString const& team = spacelPlayerState->R_Team;

			if (empBullet->R_Team != *team)
			{
				// emp
				if (AShipPawn* pawn = get<AShipPawn>())
				{
					pawn->emp(empBullet->EffectDuration, empBullet->R_Team, empBullet->PlayerIdOwner);
				}
			}
		}
	}
}

void UCustomCollisionComponent::checkGold(int32 _otherPlayerId)
{
	if (AShipPawn* shipPawn = get<AShipPawn>())
	{
		if (shipPawn->hasEffect(EEffect::Gold))
		{
			if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())))
			{
				TArray<APlayerState*> playerStates = spacelGameState->PlayerArray;
				for (auto* playerState : playerStates)
				{
					if (playerState->PlayerId == _otherPlayerId)
					{
						if (AShipPawn* otherPawn = playerState->GetPawn<AShipPawn>())
						{
							otherPawn->addEffect(EEffect::Gold);
							shipPawn->removeEffect(EEffect::Gold);
							break;
						}
					}
				}
			}
		}
	}
}

void UCustomCollisionComponent::hit(FString const& _team, int32 _playerId, class UPrimitiveComponent* _comp, int32 _index, FVector const& _otherLocation, AActor* _otherActor)
{
	AShipPawn* shipPawn = get<AShipPawn>();
	if(shipPawn == nullptr) return;
	if(shipPawn->ProtectionComponent == nullptr) return;
	if(shipPawn->SupportComponent == nullptr) return;
	if(shipPawn->DriverMeshComponent == nullptr) return;

	uint32 uniqueId { _comp->GetUniqueID() };

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

	auto lb_generic = [&](UAnimatedSpacelMeshComponent*& _component)
	{
		if (_component != nullptr && !shipPawn->hasEffect(EEffect::MetaFormProtection))
		{
			checkGold(_playerId);

			FTransform transform {};
			_component->GetInstanceTransform(_index, transform, false);

			_component->RPCNetMulticastRemove(transform.GetLocation());
			lb_addScore(EScoreType::Hit);

			// for feedback
			shipPawn->RPCClientDamageIndicator(_otherLocation);
			shipPawn->RPCClientPlayCameraShake(EImpactType::Hit);

			if (_otherActor != nullptr && _otherActor->ActorHasTag(Tags::EmpBullet))
			{
				hitEmp(_otherActor);
			}
		}
	};


	if (uniqueId == shipPawn->ProtectionComponent->GetUniqueID())
	{
		lb_generic(shipPawn->ProtectionComponent);
	}
	else if (uniqueId == shipPawn->SupportComponent->GetUniqueID())
	{
		lb_generic(shipPawn->SupportComponent);
	}
	else if (uniqueId == shipPawn->EmergencyComponent->GetUniqueID())
	{
		UAnimatedSpacelMeshComponent* tmp = Cast<UAnimatedSpacelMeshComponent>(shipPawn->EmergencyComponent);
		lb_generic(tmp);
	}
	else if (uniqueId == get()->DriverMeshComponent->GetUniqueID())
	{
		if (!shipPawn->hasEffect(EEffect::MetaFormProtection) && !shipPawn->hasEffect(EEffect::Killed))
		{
			checkGold(_playerId);
			if (m_matiereManager.IsValid())
			{
				if (ASpacelPlayerState const* spacelPlayerState = get()->GetPlayerState<ASpacelPlayerState>())
				{
					m_matiereManager.Get()->spawnMatiere(shipPawn->DriverMeshComponent->GetComponentLocation(), spacelPlayerState->R_Team);
				}
			}

			lb_addScore(EScoreType::Kill);
			shipPawn->OnKill.broadcast(shipPawn->Team.ToString(), _team);

			shipPawn->kill(_playerId);

			if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())))
			{
				if (ASpacelPlayerState const* spacelPlayerState = get()->GetPlayerState<ASpacelPlayerState>())
				{
					spacelGameState->RPCNetMulticastKill(_playerId, spacelPlayerState->PlayerId);
				}
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
						playerInfo.Key = projectileBase->PlayerIdOwner;
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