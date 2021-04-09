// Fill out your copyright notice in the Description page of Project Settings.


#include "MatiereManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Util/Tag.h"
#include <functional>

int32 getId(FString const& _team)
{
	static TArray<FString> teamNames = { "Team 1", "Team 2", "Team 3", "Team 4", "Team 5", "Team 6"};

	for (int32 i{ 0 }; i < teamNames.Num(); ++i)
	{
		if (teamNames[i] == _team)
		{
			return i;
		}
	}

	ensure(false);
	return -1;
}

// Sets default values
AMatiereManager::AMatiereManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Tags.Add(Tags::Matiere);
}

void AMatiereManager::BeginPlay()
{
	Super::BeginPlay();

	this->GetComponents<UInstancedStaticMeshComponent>(this->Matieres, true);
	ToRemoveTeams.Empty();
	for (int i{ 0 }; i < this->Matieres.Num(); ++i)
	{
		ToRemoveTeams.Add({});
	}

	// reset
	this->RU_InstanceTeam1.Empty();
	this->RU_InstanceTeam2.Empty();
	this->RU_InstanceTeam3.Empty();
	this->RU_InstanceTeam4.Empty();
	this->RU_InstanceTeam5.Empty();
	this->RU_InstanceTeam6.Empty();

	if (this->GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		PrimaryActorTick.bCanEverTick = false;
	}
	else
	{
		OnRep_InstanceTeam1();
		OnRep_InstanceTeam2();
		OnRep_InstanceTeam3();
		OnRep_InstanceTeam4();
		OnRep_InstanceTeam5();
		OnRep_InstanceTeam6();
	}
}

void AMatiereManager::Tick(float _deltaTime)
{
	Super::Tick(_deltaTime);

	auto lb_remove = [](class UInstancedStaticMeshComponent* _comp, TSet<int32>& _index, TArray<FVector>& _loc)
	{
		if (!ensure(_comp != nullptr)) return;
		if (_index.Num() == 0) return;

		_index.Sort([](int32 const& _k1, int32 const& _k2)
			{
				return _k1 > _k2;
			});

		for (int32 index : _index)
		{
			FTransform transform{};
			if(_comp->GetInstanceTransform(index, transform, true))
			{
				_loc.Remove(transform.GetLocation());
				_comp->RemoveInstance(index);
			}
		}

		_index.Empty();
	};

	const TArray<TArray<FVector>*> instance{ &this->RU_InstanceTeam1,
		&this->RU_InstanceTeam2,
		&this->RU_InstanceTeam3,
		&this->RU_InstanceTeam4,
		&this->RU_InstanceTeam5,
		&this->RU_InstanceTeam6 };

	for (int32 i{ 0 }; i < this->Matieres.Num(); ++i)
	{
		lb_remove(this->Matieres[i], this->ToRemoveTeams[i], *instance[i]);
	}
}

int AMatiereManager::depopulate(class UInstancedStaticMeshComponent* _comp, FHitResult const& _hit, TSet<int32>& _index)
{
	if (!ensure(_comp != nullptr)) return 0;
	if (_hit.Component.IsValid() && _hit.Component.Get()->GetUniqueID() != _comp->GetUniqueID()) return 0;

	FTransform world{};
	if (_comp->GetInstanceTransform(_hit.Item, world, true))
	{
		_index.Add(_hit.Item);
		return 1;
	}
	return 0;
}

int AMatiereManager::hit(FHitResult const& _hit, FString const& _team)
{
	int ret = 0;

	int32 id{ getId(_team) };

	const TArray<TArray<FVector>*> instance{ &this->RU_InstanceTeam1,
		&this->RU_InstanceTeam2,
		&this->RU_InstanceTeam3,
		&this->RU_InstanceTeam4,
		&this->RU_InstanceTeam5,
		&this->RU_InstanceTeam6 };

	static TArray<FString> teamNames = { "Team 1", "Team 2", "Team 3", "Team 4", "Team 5", "Team 6" };

	for (int32 i{ 0 }; i < this->Matieres.Num(); ++i)
	{
		if(id == getId(teamNames[i])) continue;
		ret += depopulate(this->Matieres[i], _hit, ToRemoveTeams[i]);
	}

	return ret;
}

void AMatiereManager::spawnMatiere(FVector const& _location, FString const& _team)
{
	int32 id { getId(_team) };

	const TArray<TArray<FVector>*> instance { &this->RU_InstanceTeam1,
		&this->RU_InstanceTeam2,
		&this->RU_InstanceTeam3,
		&this->RU_InstanceTeam4,
		&this->RU_InstanceTeam5,
		&this->RU_InstanceTeam6 };

	const TArray<std::function<void(void)>> fnc { std::bind(&AMatiereManager::OnRep_InstanceTeam1, this),
		std::bind(&AMatiereManager::OnRep_InstanceTeam2, this),
		std::bind(&AMatiereManager::OnRep_InstanceTeam3, this),
		std::bind(&AMatiereManager::OnRep_InstanceTeam4, this),
		std::bind(&AMatiereManager::OnRep_InstanceTeam5, this), 
		std::bind(&AMatiereManager::OnRep_InstanceTeam6, this)};

	if (id < instance.Num())
	{
		instance[id]->Add(_location);
		fnc[id]();
	}
}

void AMatiereManager::populate(class UInstancedStaticMeshComponent* _comp, TArray<FVector> const& _loc)
{
	if (!ensure(_comp != nullptr)) return;
	_comp->ClearInstances();
	for (FVector const& location : _loc)
	{
		_comp->AddInstance(FTransform{ location });
	}
}

void AMatiereManager::genericOnRep(FString const& _team)
{
	const TArray<TArray<FVector>*> instance{ &this->RU_InstanceTeam1,
			&this->RU_InstanceTeam2,
			&this->RU_InstanceTeam3,
			&this->RU_InstanceTeam4,
			&this->RU_InstanceTeam5,
			&this->RU_InstanceTeam6 };

	if (this->LocalTeam == _team) return;
	int32 id = getId(_team);
	if (id < this->Matieres.Num())
	{
		populate(this->Matieres[id], *instance[id]);
	}
}

void AMatiereManager::OnRep_InstanceTeam1()
{
	genericOnRep("Team 1");
}

void AMatiereManager::OnRep_InstanceTeam2()
{
	genericOnRep("Team 2");
}

void AMatiereManager::OnRep_InstanceTeam3()
{
	genericOnRep("Team 3");
}

void AMatiereManager::OnRep_InstanceTeam4()
{
	genericOnRep("Team 4");
}

void AMatiereManager::OnRep_InstanceTeam5()
{
	genericOnRep("Team 5");
}

void AMatiereManager::OnRep_InstanceTeam6()
{
	genericOnRep("Team 6");
}

void AMatiereManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMatiereManager, RU_InstanceTeam1);
	DOREPLIFETIME(AMatiereManager, RU_InstanceTeam2);
	DOREPLIFETIME(AMatiereManager, RU_InstanceTeam3);
	DOREPLIFETIME(AMatiereManager, RU_InstanceTeam4);
	DOREPLIFETIME(AMatiereManager, RU_InstanceTeam5);
	DOREPLIFETIME(AMatiereManager, RU_InstanceTeam6);
}

