// Fill out your copyright notice in the Description page of Project Settings.


#include "Pirate.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DataAsset/PirateDataAsset.h"
#include "Util/SimplyXml.h"
#include "Util/Tag.h"

// Sets default values
APirate::APirate()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RedCube = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RedCube"));
	RootComponent = RedCube;

	Voxels = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Voxels"));
	Voxels->SetupAttachment(RedCube);

	Tags.Add(Tags::BlockingActor);
}

// Called when the game starts or when spawned
void APirate::BeginPlay()
{
	Super::BeginPlay();
	
	BuildShip();

	if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
        if (this->Voxels != nullptr)
        {
            this->Voxels->OnComponentHit.AddDynamic(this, &APirate::OnVoxelsHit);
        }
        if (this->RedCube != nullptr)
        {
            this->RedCube->OnComponentHit.AddDynamic(this, &APirate::OnRedCubeHit);
        }
	}
}

// Called every frame
void APirate::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APirate::applyHit(TArray<int32>& _instance)
{

}

void APirate::hit(class AShipPawn* _pawn, FString const& _team)
{

}

void APirate::OnVoxelsHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{

}

void APirate::OnRedCubeHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{

}

void APirate::BuildShip()
{
    if(this->DataAsset == nullptr) return;
    if(this->Voxels == nullptr) return;

    SimplyXml::FContainer<FVector> locationInformation { "Location" };
    SimplyXml::FReader reader { FPaths::ProjectDir() + this->DataAsset->XmlPath };
    reader.read(locationInformation);

    this->Voxels->ClearInstances();
    TArray<FVector> locations;
    for (FVector loc : locationInformation.Values)
    {
        locations.Add(loc);
    }

    this->Voxels->SetEnableGravity(false);
    for (auto const& loc : locations)
    {
        FTransform voxelTransform{};
        voxelTransform.SetLocation(loc);
        this->Voxels->AddInstance(voxelTransform);
    }
}