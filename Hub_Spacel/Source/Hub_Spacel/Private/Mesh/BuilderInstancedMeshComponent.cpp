// Fill out your copyright notice in the Description page of Project Settings.


#include "BuilderInstancedMeshComponent.h"
#include "Util/SimplyXml.h"

UBuilderInstancedMeshComponent::UBuilderInstancedMeshComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}


UBuilderInstancedMeshComponent::UBuilderInstancedMeshComponent(FVTableHelper& _helper)
    : Super(_helper)
{
}

UBuilderInstancedMeshComponent::~UBuilderInstancedMeshComponent()
{
}

void UBuilderInstancedMeshComponent::Export() const
{
    TArray<FVector_NetQuantize> locations;
    locations.Reserve(this->ThresholdBonus);
    for (int i = 0; (i < this->ThresholdBonus) && i < this->Locations.Num(); ++i)
    {
        locations.Add(this->Locations[i]);
    }

    TArray<FVector_NetQuantize> locationsBonus;
    locationsBonus.Reserve(this->MaxVoxel - this->ThresholdBonus);

    for (int i = this->ThresholdBonus; (i < this->MaxVoxel) && i < this->Locations.Num(); ++i)
    {
        locationsBonus.Add(this->Locations[i]);
    }

    SimplyXml::FWriter writer { FPaths::ProjectDir() + this->Path };
    writer.write(SimplyXml::FContainer<FVector_NetQuantize>{ "Location", locations},
        SimplyXml::FContainer<FVector_NetQuantize>{ "Bonus", locationsBonus});
}