// Fill out your copyright notice in the Description page of Project Settings.


#include "MatiereManager.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameModeBase.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Util/Tag.h"

// Sets default values
AMatiereManager::AMatiereManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MatieresTeam1 = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("MatieresTeam1"));
	RootComponent = MatieresTeam1;

	MatieresTeam2 = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("MatieresTeam2"));
	if (!ensure(MatieresTeam2 != nullptr)) return;
	MatieresTeam2->SetupAttachment(RootComponent);

	MatieresTeam3 = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("MatieresTeam3"));
	if (!ensure(MatieresTeam3 != nullptr)) return;
	MatieresTeam3->SetupAttachment(RootComponent);

	MatieresTeam4 = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("MatieresTeam4"));
	if (!ensure(MatieresTeam4 != nullptr)) return;
	MatieresTeam4->SetupAttachment(RootComponent);

	MatieresTeam5 = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("MatieresTeam5"));
	if (!ensure(MatieresTeam5 != nullptr)) return;
	MatieresTeam5->SetupAttachment(RootComponent);

	MatieresTeam6 = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("MatieresTeam6"));
	if (!ensure(MatieresTeam6 != nullptr)) return;
	MatieresTeam6->SetupAttachment(RootComponent);

	Tags.Add(Tags::Matiere);
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

	lb_remove(this->MatieresTeam1, m_toRemoveTeam1, this->RU_InstanceTeam1);
	lb_remove(this->MatieresTeam2, m_toRemoveTeam2, this->RU_InstanceTeam2);
	lb_remove(this->MatieresTeam3, m_toRemoveTeam3, this->RU_InstanceTeam3);
	lb_remove(this->MatieresTeam4, m_toRemoveTeam4, this->RU_InstanceTeam4);
	lb_remove(this->MatieresTeam5, m_toRemoveTeam5, this->RU_InstanceTeam5);
	lb_remove(this->MatieresTeam6, m_toRemoveTeam6, this->RU_InstanceTeam6);
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
	if (_team != "Team 1")
	{
		ret += depopulate(this->MatieresTeam1, _hit, m_toRemoveTeam1);
	}
	if (_team != "Team 2")
	{
		ret += depopulate(this->MatieresTeam2, _hit, m_toRemoveTeam2);
	}
	if (_team != "Team 3")
	{
		ret += depopulate(this->MatieresTeam3, _hit, m_toRemoveTeam3);
	}
	if (_team != "Team 4")
	{
		ret += depopulate(this->MatieresTeam4, _hit, m_toRemoveTeam4);
	}
	if (_team != "Team 5")
	{
		ret += depopulate(this->MatieresTeam5, _hit, m_toRemoveTeam5);
	}
	if (_team != "Team 6")
	{
		ret += depopulate(this->MatieresTeam6, _hit, m_toRemoveTeam6);
	}

	return ret;
}

void AMatiereManager::spawnMatiere(FVector const& _location, FString const& _team)
{
	if (_team == "Team 1")
	{
		this->RU_InstanceTeam1.Add(_location);
		OnRep_InstanceTeam1();
	}
	else if (_team == "Team 2")
	{
		this->RU_InstanceTeam2.Add(_location);
		OnRep_InstanceTeam2();
	}
	else if (_team == "Team 3")
	{
		this->RU_InstanceTeam3.Add(_location);
		OnRep_InstanceTeam3();
	}
	else if (_team == "Team 4")
	{
		this->RU_InstanceTeam4.Add(_location);
		OnRep_InstanceTeam4();
	}
	else if (_team == "Team 5")
	{
		this->RU_InstanceTeam5.Add(_location);
		OnRep_InstanceTeam5();
	}
	else if (_team == "Team 6")
	{
		this->RU_InstanceTeam6.Add(_location);
		OnRep_InstanceTeam6();
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

void AMatiereManager::OnRep_InstanceTeam1()
{
	if(this->LocalTeam == "Team 1") return;
	populate(this->MatieresTeam1, this->RU_InstanceTeam1);
}

void AMatiereManager::OnRep_InstanceTeam2()
{
	if (this->LocalTeam == "Team 2") return;
	populate(this->MatieresTeam2, this->RU_InstanceTeam2);
}

void AMatiereManager::OnRep_InstanceTeam3()
{
	if (this->LocalTeam == "Team 3") return;
	populate(this->MatieresTeam3, this->RU_InstanceTeam3);
}

void AMatiereManager::OnRep_InstanceTeam4()
{
	if (this->LocalTeam == "Team 4") return;
	populate(this->MatieresTeam4, this->RU_InstanceTeam4);
}

void AMatiereManager::OnRep_InstanceTeam5()
{
	if (this->LocalTeam == "Team 5") return;
	populate(this->MatieresTeam5, this->RU_InstanceTeam5);
}

void AMatiereManager::OnRep_InstanceTeam6()
{
	if (this->LocalTeam == "Team 6") return;
	populate(this->MatieresTeam6, this->RU_InstanceTeam6);
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

