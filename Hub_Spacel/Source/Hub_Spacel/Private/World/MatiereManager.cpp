// Fill out your copyright notice in the Description page of Project Settings.


#include "MatiereManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AMatiereManager::AMatiereManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Matieres = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Matieres"));
	RootComponent = Matieres;

	Tags.Add("Matiere");
}

void AMatiereManager::BeginPlay()
{
	Super::BeginPlay();
	if (this->GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		PrimaryActorTick.bCanEverTick = false;
	}
	else
	{
		OnRep_Instance();
	}
}

void AMatiereManager::Tick(float _deltaTime)
{
	Super::Tick(_deltaTime);

	if (!ensure(this->Matieres != nullptr)) return;

	if (m_instanceToRemove.Num() != 0)
	{
		m_instanceToRemove.Sort([](int32 const& _k1, int32 const& _k2)
			{
				return _k1 > _k2;
			});

		for (int32 index : m_instanceToRemove)
		{
			FTransform transform{};
			this->Matieres->GetInstanceTransform(index, transform, true);

			RU_Instance.Remove(transform.GetLocation());
			this->Matieres->RemoveInstance(index);
		}
		m_instanceToRemove.Empty();
	}
}

int AMatiereManager::hit(FHitResult const& _hit, FString const& _team)
{
	if (!ensure(this->Matieres != nullptr)) return 0;

	FTransform world {};
	if (this->Matieres->GetInstanceTransform(_hit.Item, world, true))
	{
		FVector const& location = world.GetLocation();
		if(m_spawnByTeam.Contains(location) && 
			m_spawnByTeam[location] != _team)
		{
			m_spawnByTeam.Remove(location);
			m_instanceToRemove.Add(_hit.Item);
			return 1;
		}
	}
	return 0;
}

void AMatiereManager::spawnMatiere(FVector const& _location, FString const& _team)
{
	if (!ensure(this->Matieres != nullptr)) return;

	if (!m_spawnByTeam.Contains(_location))
	{
		this->RU_Instance.Add(_location);
		m_spawnByTeam.Add(TTuple<FVector, FString>{_location, _team});
		OnRep_Instance();
	}
}

void AMatiereManager::OnRep_Instance()
{
	if (!ensure(this->Matieres != nullptr)) return;

	this->Matieres->ClearInstances();
	for (FVector const& location : RU_Instance)
	{
		this->Matieres->AddInstance(FTransform{ location });
	}
}


void AMatiereManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMatiereManager, RU_Instance);
}

