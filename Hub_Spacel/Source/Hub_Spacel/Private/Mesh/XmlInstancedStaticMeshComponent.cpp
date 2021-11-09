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
    SimplyXml::FContainer<FVector_NetQuantize> baseLocationInformation { "Location" };
    SimplyXml::FContainer<FVector_NetQuantize> bonusLocationInformation{ "Bonus" };

    SimplyXml::FReader reader{ FPaths::ProjectDir() + this->Path };

    reader.read(baseLocationInformation, bonusLocationInformation);

    m_baseLocations = baseLocationInformation.Values;
    m_bonusLocations = bonusLocationInformation.Values;

    // sort location by distance with origin
    using TVec = FVector_NetQuantize const&;
    auto lbSort = [](TVec _v1, TVec _v2) -> bool
    {
        auto d1 = FVector::Distance(_v1, { 0.0f, 0.0f, 0.0f });
        auto d2 = FVector::Distance(_v2, { 0.0f, 0.0f, 0.0f });
        return d1 < d2;
    };

    m_baseLocations.Sort(lbSort);
    m_bonusLocations.Sort(lbSort);
}

void UXmlInstancedStaticMeshComponent::InitLocations(bool _useBonus)
{
    this->Locations = m_baseLocations;
    if (_useBonus)
    {
        this->Locations.Append(m_bonusLocations);
    }
}

void UXmlInstancedStaticMeshComponent::Add(FVector_NetQuantize const& _location)
{
    this->Locations.Add(_location);
    this->AddInstance(FTransform(_location));
}

int UXmlInstancedStaticMeshComponent::Remove(FVector_NetQuantize const& _location)
{
    bool isRemoved { false };
    int i {0};
    for (auto loc : this->Locations)
    {
        if (loc == _location)
        {
            this->Locations.RemoveAt(i);
            isRemoved = true;
            break;
        }
        ++i;
    }

    if (isRemoved)
    {
        this->RemoveInstance(i);
        return i;
    }
    else
    {
        return -1;
    }
}

void UXmlInstancedStaticMeshComponent::resetBuild()
{
    this->ClearInstances();
    for (auto loc : this->Locations)
    {
        this->AddInstance(FTransform { loc });
    }
}