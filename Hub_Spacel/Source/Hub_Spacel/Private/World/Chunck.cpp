// Fill out your copyright notice in the Description page of Project Settings.


#include "Chunck.h"
#include "Noise/SpacelNoise.h"
#include "Components/InstancedStaticMeshComponent.h"

// Sets default values
AChunck::AChunck()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Voxels = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Voxels"));
	Voxels->SetCollisionProfileName("BlockAll");
	Voxels->OnComponentHit.AddDynamic(this, &AChunck::OnComponentHit);
	RootComponent = Voxels;

	Tags.Add("BlockingActor");
}

bool AChunck::init(FVector2D const& _bornX, FVector2D const& _bornY, FVector2D const& _bornZ, int32 _cubeSize)
{
	this->BornX = _bornX;
	this->BornY = _bornY;
	this->BornZ = _bornZ;
	this->CubeSize = _cubeSize;

	return generateChunck();
}

// Called when the game starts or when spawned
void AChunck::BeginPlay()
{
	Super::BeginPlay();

	if (!ensure(this->Voxels != nullptr)) return;
	if (this->Voxels->GetInstanceCount() == 0)
	{
		this->Destroy();
	}
	else
	{
		this->Voxels->SetStaticMesh(this->VoxelStaticMesh);
		this->Voxels->SetEnableGravity(false);
	}
}

float AChunck::getNoise(FVector const& _location) const
{
	// increase float broke bloc, increase int (octave) add more bloc
	return SpacelNoise::getInstance()->getOctaveNoise((_location.X + this->BornX.X) * 0.00007f, (_location.Y + this->BornY.X) * 0.00007f, (_location.Z + this->BornZ.X) * 0.00007f, 2);
}

bool AChunck::generateChunck()
{
	int maxX = (this->BornX.Y - this->BornX.X) / this->CubeSize;
	int maxY = (this->BornY.Y - this->BornY.X) / this->CubeSize;
	int maxZ = (this->BornZ.Y - this->BornZ.X) / this->CubeSize;
	int size = maxX * maxY * maxZ;

	bool ret = false;

	for (int x = 0; x < maxX; ++x)
	{
		for (int y = 0; y < maxY; ++y)
		{
			for (int z = 0; z < maxZ; ++z)
			{
				FVector location { (float)(x * this->CubeSize), (float)(y * this->CubeSize), (float)(z * this->CubeSize) };
				float noise { getNoise(location) };

				if (noise > .75f)
				{
					FTransform voxelTransform {};
					voxelTransform.SetLocation(location);
					this->Voxels->AddInstance(voxelTransform);
					ret = true;
				}
			}
		}
	}
	return ret;
}

void AChunck::OnComponentHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
	if (!ensure(this->Voxels != nullptr)) return;
	this->Voxels->RemoveInstance(_hit.Item);
	if (this->Voxels->GetInstanceCount() == 0)
	{
		this->Destroy();
	}
}
