// Fill out your copyright notice in the Description page of Project Settings.


#include "Chunck.h"
#include "Noise/SpacelNoise.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "World/FogActor.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AChunck::AChunck()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Voxels = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Voxels"));
	Voxels->SetCollisionProfileName("BlockAll");
	RootComponent = Voxels;

	Tags.Add("BlockingActor");
}

void AChunck::dmg(FHitResult const& _info)
{
	Super::dmg(_info);

	if (_info.Item >= 0)
	{
		if (!m_dmg.Contains(_info.Item))
		{
			m_dmg.Add(_info.Item);
		}

		m_dmg[_info.Item]++;
	}
}

void AChunck::init(int _chunckSize, int32 _cubeSize)
{
	this->R_ChunckSize = _chunckSize;
	this->R_CubeSize = _cubeSize;
}

// Called when the game starts or when spawned
void AChunck::BeginPlay()
{
	Super::BeginPlay();

	if (!ensure(this->Voxels != nullptr)) return;

	// server side
	if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		generateChunck(true);

		if (this->Voxels->GetInstanceCount() == 0)
		{
			this->Destroy();
		}
		else
		{
			this->Voxels->OnComponentHit.AddDynamic(this, &AChunck::OnComponentHit);
		}
	}
	else
	{
		generateChunck(false);
		// if we reconnect a player, need to remove instance already destroyed
		OnRep_RemoveInstance();
	}

	this->Voxels->SetStaticMesh(this->VoxelStaticMesh);
	this->Voxels->SetEnableGravity(false);
}

float AChunck::getNoise(FVector const& _location, FVector2D const& _bornX, FVector2D const& _bornY, FVector2D const& _bornZ) const
{
	// increase float broke bloc, increase int (octave) add more bloc
	return SpacelNoise::getInstance()->getOctaveNoise((_location.X + _bornX.X) * 0.00007f, (_location.Y + _bornY.X) * 0.00007f, (_location.Z + _bornZ.X) * 0.00007f, 2);
}

void AChunck::generateChunck(bool _isServer)
{
	FVector actorLocation{ this->GetActorLocation() };
	FVector2D bornX = FVector2D(actorLocation.X, actorLocation.X + this->R_ChunckSize);
	FVector2D bornY = FVector2D(actorLocation.Y, actorLocation.Y + this->R_ChunckSize);
	FVector2D bornZ = FVector2D(actorLocation.Z, actorLocation.Z + this->R_ChunckSize);

	int maxX { (int)((bornX.Y - bornX.X) / this->R_CubeSize) };
	int maxY { (int)((bornY.Y - bornY.X) / this->R_CubeSize) };
	int maxZ { (int)((bornZ.Y - bornZ.X) / this->R_CubeSize) };
	int size { maxX * maxY * maxZ };

	UWorld* const world{ this->GetWorld() };
	if (!ensure(world != nullptr)) return;

	for (int x = 0; x < maxX; ++x)
	{
		for (int y = 0; y < maxY; ++y)
		{
			for (int z = 0; z < maxZ; ++z)
			{
				FVector location { (float)(x * this->R_CubeSize), (float)(y * this->R_CubeSize), (float)(z * this->R_CubeSize) };
				float noise { getNoise(location, bornX, bornY, bornZ) };

				if (noise > .75f)
				{
					this->Voxels->AddInstance(FTransform { location });
				}
				else if (_isServer && noise > 0.60 && noise < 0.600001)
				{
					world->SpawnActor<AFogActor>(this->FogClass, FTransform { actorLocation + location });
				}
			}
		}
	}
}

void AChunck::OnComponentHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
	// only trigger on server (because only register on server)
	dmg(_hit);
	applyDmg();
}

void AChunck::applyDmg()
{
	if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
		if (m_dmg.Num() != 0)
		{
			TArray<int32> tmpArray{};
			m_dmg.KeySort([](int32 const& _k1, int32 const& _k2)
				{
					return _k1 < _k2;
				});

			for (auto const& pair : m_dmg)
			{
				this->Voxels->RemoveInstance(pair.Key);
				tmpArray.Add(pair.Key);
			}
			m_dmg.Empty();

			if (this->Voxels->GetInstanceCount() == 0)
			{
				this->Destroy();
			}
			else
			{
				RU_RemoveIndex.Append(tmpArray);
			}
		}
	}
}

void AChunck::OnRep_RemoveInstance()
{
	if (!ensure(this->Voxels != nullptr)) return;

	for (int32 i = m_countRemovedIndex; i < RU_RemoveIndex.Num(); ++i)
	{
		this->Voxels->RemoveInstance(RU_RemoveIndex[i]);
	}

	m_countRemovedIndex = RU_RemoveIndex.Num();
}

void AChunck::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AChunck, R_ChunckSize);
	DOREPLIFETIME(AChunck, R_CubeSize);
	DOREPLIFETIME(AChunck, RU_RemoveIndex);
}