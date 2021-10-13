// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacelInstancedMeshComponent.h"
#include "DataAsset/FormDataAsset.h"

void USpacelInstancedMeshComponent::UseForm(EFormType _type)
{
    for (auto* dataAsset : this->Forms)
    {
        if (dataAsset != nullptr && dataAsset->Type == _type)
        {
            this->SetStaticMesh(dataAsset->StaticMesh);
            if(m_loaded.find(_type) != m_loaded.end())
            {
                RPCNetMulticastUseForm(_type, m_removedLocations.Num());
            }
            else
            {
                // read xml
                this->Path = dataAsset->Path;
                // TO DO check if we use bonus
                Read(this->UseBonus);
                RPCNetMulticastAddForm(_type, this->Locations, m_removedLocations.Num());
            }
            return;
        }
    }

    _type != EFormType::Base ? UseForm(EFormType::Base) : ensure(false);
}

void USpacelInstancedMeshComponent::RPCNetMulticastUseForm_Implementation(EFormType _type, uint8 _ignoreLast)
{
    if (m_loaded.find(_type) != m_loaded.end())
    {
        this->Locations = m_loaded[_type];
        this->Locations.RemoveAt(this->Locations.Num() - _ignoreLast, _ignoreLast);
        resetBuild();
    }
    else
    {
        // TO DO : fallback
    }
}

void USpacelInstancedMeshComponent::RPCNetMulticastAddForm_Implementation(EFormType _type, TArray<FVector_NetQuantize> const& _locations, uint8 _ignoreLast)
{
    m_loaded.insert({ _type , _locations });

    this->Locations = _locations;
    this->Locations.RemoveAt(this->Locations.Num() - _ignoreLast, _ignoreLast);
    resetBuild();
}