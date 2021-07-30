// Fill out your copyright notice in the Description page of Project Settings.


#include "Pirate.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DataAsset/PirateDataAsset.h"
#include "Util/SimplyXml.h"
#include "Util/Tag.h"
#include "Util/DebugScreenMessage.h"
#include "Gameplay/Bullet/ProjectileBase.h"

// Sets default values
APirate::APirate()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RedCube = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RedCube"));
	RootComponent = RedCube;

    Base = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Base"));
    Base->SetupAttachment(RedCube);

    Addon = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Addon"));
    Addon->SetupAttachment(RedCube);
}

// Called when the game starts or when spawned
void APirate::BeginPlay()
{
	Super::BeginPlay();
    Tags.Add(Tags::Pirate);
	
	BuildShip();

	if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
	{
        if (this->Base != nullptr)
        {
            this->Base->OnComponentHit.AddDynamic(this, &APirate::OnVoxelsHit);
        }
        if (this->Addon != nullptr)
        {
            this->Addon->OnComponentHit.AddDynamic(this, &APirate::OnVoxelsHit);
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

void APirate::OnVoxelsHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
    if (this->Base != nullptr && this->Base->GetUniqueID() == _hitComp->GetUniqueID())
    {
        RPCNetMulticastHit(_hit.Item, (uint8)EComponentType::Base);
    }
    else if (this->Addon != nullptr && this->Addon->GetUniqueID() == _hitComp->GetUniqueID())
    {
        RPCNetMulticastHit(_hit.Item, (uint8)EComponentType::Addon);
    }
}

void APirate::OnRedCubeHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
    if (_otherActor != nullptr)
    {
        if (AProjectileBase* projectile = Cast<AProjectileBase>(_otherActor))
        {
            OnKilledDelegate.broadcast(projectile->R_Team);
        }
    }
    this->Destroy();
}

void APirate::RPCNetMulticastHit_Implementation(int32 _index, uint8 _type)
{
    if (this->Base != nullptr && _type == (uint8)EComponentType::Base)
    {
        this->Base->RemoveInstance(_index);
    }
    else if (this->Addon != nullptr && _type == (uint8)EComponentType::Addon)
    {
        this->Addon->RemoveInstance(_index);
    }
}

void APirate::BuildShip()
{
    if(this->DataAsset == nullptr) return;

    auto lb = [&](FString&& _name, UInstancedStaticMeshComponent* _comp)
    {
        if(_comp == nullptr) return;
        SimplyXml::FContainer<FVector> locationInformation{ _name };
        SimplyXml::FReader reader{ FPaths::ProjectDir() + this->DataAsset->XmlPath };
        reader.read(locationInformation);

        _comp->ClearInstances();
        TArray<FVector> locations;
        for (FVector loc : locationInformation.Values)
        {
            locations.Add(loc);
        }

        _comp->SetEnableGravity(false);
        for (auto const& loc : locations)
        {
            FTransform voxelTransform{};
            voxelTransform.SetLocation(loc);
            _comp->AddInstance(voxelTransform);
        }
    };

    lb("Base", this->Base);
    lb("Addon", this->Addon);
}