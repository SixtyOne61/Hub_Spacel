// Fill out your copyright notice in the Description page of Project Settings.


#include "XmlInstancedStaticMeshComponent.h"

UXmlInstancedStaticMeshComponent::UXmlInstancedStaticMeshComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}


UXmlInstancedStaticMeshComponent::UXmlInstancedStaticMeshComponent(FVTableHelper& _helper)
    : Super(_helper)
{
}

UXmlInstancedStaticMeshComponent::~UXmlInstancedStaticMeshComponent()
{
}

void UXmlInstancedStaticMeshComponent::Read()
{

}

void UXmlInstancedStaticMeshComponent::Add(FVector_NetQuantize const& _location)
{
    this->Locations.Add(_location);
    this->AddInstance(FTransform(_location));

    // TO DO manage online
}

void UXmlInstancedStaticMeshComponent::Remove(FVector_NetQuantize const& _location)
{
    int32 id = this->Locations.Remove(_location);
    this->RemoveInstance(id);

    // TO DO manage online
}