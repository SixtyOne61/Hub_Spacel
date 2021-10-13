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
                RPCNetMulticastUseForm(_type);
            }
            else
            {
                // read xml
                this->Path = dataAsset->Path;
                // TO DO check if we use bonus
                Read(this->UseBonus);
                RPCNetMulticastAddForm(_type, this->Locations);
            }
        }
    }

    _type != EFormType::Base ? UseForm(EFormType::Base) : ensure(false);
}

void USpacelInstancedMeshComponent::RPCNetMulticastUseForm_Implementation(EFormType _type)
{
    if (m_loaded.find(_type) != m_loaded.end())
    {
        this->Locations = m_loaded[_type];
        resetBuild();
    }
    else
    {
        // TO DO : fallback
    }
}

void USpacelInstancedMeshComponent::RPCNetMulticastAddForm_Implementation(EFormType _type, TArray<FVector_NetQuantize> const& _locations)
{
    this->Locations = _locations;
    m_loaded.insert({ _type , this->Locations });
    resetBuild();
}