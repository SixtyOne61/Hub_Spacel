// Fill out your copyright notice in the Description page of Project Settings.

#include "MeshFactory.h"
#include "DataAsset/MeshDataAsset.h"
#include "Mesh/SpacelProceduralMeshComponent.h"

// Sets default values
AMeshFactory::AMeshFactory()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    Voxel = CreateDefaultSubobject<USpacelProceduralMeshComponent>(TEXT("VoxelEnvironment"));
    if (!ensure(Voxel != nullptr)) return;
    RootComponent = Voxel;
}

void AMeshFactory::GenerateVoxel()
{
    if (!ensure(this->Voxel != nullptr)) return;
    if (!ensure(this->MeshDataAsset != nullptr)) return;

    this->Voxel->generateVoxelMesh(this->MeshDataAsset->EnvironmentVoxelSize);
    this->Voxel->SetMaterial(0, this->MeshDataAsset->MaterialEnvironmentVoxel);
}

