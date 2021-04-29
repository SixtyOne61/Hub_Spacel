// Fill out your copyright notice in the Description page of Project Settings.


#include "NinePackActor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Util/Tag.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ANinePackActor::ANinePackActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Voxels = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Voxels"));
	Voxels->SetCollisionProfileName("BlockAll");
	RootComponent = Voxels;

	Tags.Add(Tags::BlockingActor);
}

// Called when the game starts or when spawned
void ANinePackActor::BeginPlay()
{
	Super::BeginPlay();
	
	if (!ensure(this->Voxels != nullptr)) return;

	this->Voxels->AddInstance(FTransform(FVector(0.0f, -123.0f, -123.0f)));
	this->Voxels->AddInstance(FTransform(FVector(0.0f, -43.0f, -123.0f)));
	this->Voxels->AddInstance(FTransform(FVector(0.0f, 43.0f, -123.0f)));
	this->Voxels->AddInstance(FTransform(FVector(0.0f, 123.0f, -123.0f)));

	this->Voxels->AddInstance(FTransform(FVector(0.0f, -123.0f, -43.0f)));
	this->Voxels->AddInstance(FTransform(FVector(0.0f, -43.0f, -43.0f)));
	this->Voxels->AddInstance(FTransform(FVector(0.0f, 43.0f, -43.0f)));
	this->Voxels->AddInstance(FTransform(FVector(0.0f, 123.0f, -43.0f)));

	this->Voxels->AddInstance(FTransform(FVector(0.0f, -123.0f, 43.0f)));
	this->Voxels->AddInstance(FTransform(FVector(0.0f, -43.0f, 43.0f)));
	this->Voxels->AddInstance(FTransform(FVector(0.0f, 43.0f, 43.0f)));
	this->Voxels->AddInstance(FTransform(FVector(0.0f, 123.0f, 43.0f)));

	this->Voxels->AddInstance(FTransform(FVector(0.0f, -123.0f, 123.0f)));
	this->Voxels->AddInstance(FTransform(FVector(0.0f, -43.0f, 123.0f)));
	this->Voxels->AddInstance(FTransform(FVector(0.0f, 43.0f, 123.0f)));
	this->Voxels->AddInstance(FTransform(FVector(0.0f, 123.0f, 123.0f)));

	this->Voxels->AddInstance(FTransform(FVector(-86.0f, 0.0f, 0.0f)));

	// server side
	if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		this->Voxels->OnComponentHit.AddDynamic(this, &ANinePackActor::OnComponentHit);
	}
	else
	{
		// if we reconnect a player, need to remove instance already destroyed
		OnRep_RemoveInstance();
	}
}

void ANinePackActor::applyHit(TArray<int32>& _instance)
{
	Super::applyHit(_instance);

	_instance.Sort([](int32 const& _a, int32 const& _b)
		{
			return _a > _b;
		});

	for (int32 id : _instance)
	{
		this->Voxels->RemoveInstance(id);
	}

	if (this->Voxels->GetInstanceCount() == 0)
	{
		this->Destroy();
	}
	else
	{
		RU_RemoveIndex.Append(_instance);
	}
}

void ANinePackActor::OnComponentHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
	// only trigger on server (because only register on server)
	TArray<int32> instance;
	instance.Add(_hit.Item);
	applyHit(instance);
}

void ANinePackActor::OnRep_RemoveInstance()
{
	if (!ensure(this->Voxels != nullptr)) return;

	for (int32 i = m_countRemovedIndex; i < RU_RemoveIndex.Num(); ++i)
	{
		this->Voxels->RemoveInstance(RU_RemoveIndex[i]);
	}

	m_countRemovedIndex = RU_RemoveIndex.Num();
}

void ANinePackActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANinePackActor, RU_RemoveIndex);
}
