// Fill out your copyright notice in the Description page of Project Settings.


#include "XmlInstancedStaticMeshComponent.h"
#include "Util/SimplyXml.h"

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
    SimplyXml::FContainer<FVector_NetQuantize> locationInformation { "Location" };
    SimplyXml::FReader reader { FPaths::ProjectDir() + this->Path };
    reader.read(locationInformation);

    this->Locations = locationInformation.Values;
    resetBuild();
}

void UXmlInstancedStaticMeshComponent::Add(FVector_NetQuantize const& _location)
{
    this->Locations.Add(_location);
    this->AddInstance(FTransform(_location));

    // TO DO manage online
}

void UXmlInstancedStaticMeshComponent::Remove(FVector_NetQuantize const& _location)
{
    int i {0};
    for (auto loc : this->Locations)
    {
        if (loc == _location)
        {
            this->Locations.RemoveAt(i);
            break;
        }
        ++i;
    }

    this->RemoveInstance(i);
    // TO DO manage online
}

void UXmlInstancedStaticMeshComponent::resetBuild()
{
    this->ClearInstances();
    for (auto loc : this->Locations)
    {
        this->AddInstance(FTransform { loc });
    }

    // TO DO manage online
}