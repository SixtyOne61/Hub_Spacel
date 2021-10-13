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

void UXmlInstancedStaticMeshComponent::Read(bool _useBonus)
{
    SimplyXml::FContainer<FVector_NetQuantize> baseLocationInformation { "Location" };
    SimplyXml::FContainer<FVector_NetQuantize> bonusLocationInformation{ "Bonus" };

    SimplyXml::FReader reader{ FPaths::ProjectDir() + this->Path };

    reader.read(baseLocationInformation, bonusLocationInformation);

    m_baseLocations = baseLocationInformation.Values;
    m_bonusLocations = bonusLocationInformation.Values;

    this->Locations = m_baseLocations;
    if (_useBonus)
    {
        this->Locations.Append(m_bonusLocations);
    }

    resetBuild();
}

void UXmlInstancedStaticMeshComponent::Add(FVector_NetQuantize const& _location)
{
    this->Locations.Add(_location);
    this->AddInstance(FTransform(_location));
}

int UXmlInstancedStaticMeshComponent::Remove(FVector_NetQuantize const& _location)
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
    return i;
}

void UXmlInstancedStaticMeshComponent::resetBuild()
{
    this->ClearInstances();
    for (auto loc : this->Locations)
    {
        this->AddInstance(FTransform { loc });
    }
}