// Fill out your copyright notice in the Description page of Project Settings.


#include "PawnCapture.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Util/SimplyXml.h"
#include "DataAsset/SetupAttributeDataAsset.h"

// Sets default values
APawnCapture::APawnCapture()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void APawnCapture::BuildShip()
{
    auto lb_readXml = [](USetupAttributeDataAsset* _dataAsset, TArray<FVector_NetQuantize>& _out, FString&& _name)
    {
        if (!ensure(_dataAsset != nullptr)) return;
        FString const& path { _dataAsset->HeavyPath };

        SimplyXml::FContainer<FVector> locationInformation{ _name };
        SimplyXml::FReader reader{ FPaths::ProjectDir() + path };
        reader.read(locationInformation);

        _out.Empty();
        for (FVector loc : locationInformation.Values)
        {
            _out.Add(loc);
        }
    };

    auto lb_build = [](UInstancedStaticMeshComponent * &_mesh, UStaticMeshDataAsset * _staticMesh, TArray<FVector_NetQuantize> const& _locations)
    {
        if (_mesh && _staticMesh)
        {
            _mesh->ClearInstances();
            _mesh->SetStaticMesh(_staticMesh->StaticMesh);
            _mesh->SetEnableGravity(false);

            for (auto const& _location : _locations)
            {
                FTransform voxelTransform{};
                voxelTransform.SetLocation(_location);
                _mesh->AddInstance(voxelTransform);
            }
        }
    };

    TArray<FVector_NetQuantize> locations {};

    lb_readXml(this->WeaponDataAsset, locations, "Location");
    lb_build(this->WeaponMeshComponent, this->WeaponDataAsset, locations);

    lb_readXml(this->ProtectionDataAsset, locations, "Location");
    lb_build(this->ProtectionMeshComponent, this->ProtectionDataAsset, locations);

    lb_readXml(this->SupportDataAsset, locations, "Location");
    lb_build(this->SupportMeshComponent, this->SupportDataAsset, locations);
}

// Called when the game starts or when spawned
void APawnCapture::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APawnCapture::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

