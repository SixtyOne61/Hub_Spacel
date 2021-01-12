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
	PrimaryActorTick.bCanEverTick = false;

	Matieres = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Matieres"));
	RootComponent = Matieres;

	Tags.Add("Matiere");
}

void AMatiereManager::spawnMatiere(FVector const& _location, FString const& _team)
{
	if (!ensure(this->Matieres != nullptr)) return;

	if (!m_spawnByTeam.Contains(_location))
	{
		this->Matieres->AddInstance(FTransform{ _location });
		m_spawnByTeam.Add(TTuple<FVector, FString>{_location, _team});
		this->RU_AddInstance = _location;
	}
}

void AMatiereManager::OnRep_AddInstance()
{
	if (!ensure(this->Matieres != nullptr)) return;
	this->Matieres->AddInstance(FTransform{ RU_AddInstance });
}

void AMatiereManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMatiereManager, RU_AddInstance);
}

