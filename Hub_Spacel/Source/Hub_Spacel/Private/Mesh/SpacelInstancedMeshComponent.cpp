// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacelInstancedMeshComponent.h"
#include "DataAsset/FormDataAsset.h"

void USpacelInstancedMeshComponent::UseForm(EFormType _type, bool _refresh)
{
    for (auto* dataAsset : this->Forms)
    {
        if (dataAsset != nullptr && dataAsset->Type == _type)
        {
            if (_refresh)
            {
                m_removedLocations.Empty();
            }

            this->SetStaticMesh(dataAsset->StaticMesh);
            this->SetEnableGravity(false);
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

    _type != EFormType::Base ? UseForm(EFormType::Base, _refresh) : ensure(false);
}

uint8 USpacelInstancedMeshComponent::Repair(uint8 _nbRepair)
{
    while (_nbRepair > 0 && m_removedLocations.Num() != 0)
    {
        RPCNetMulticastAdd(m_removedLocations.Pop());
        --_nbRepair;
    }

    return _nbRepair;
}

void USpacelInstancedMeshComponent::RemoveRandom(uint8 _nbToRemove)
{
    while (_nbToRemove > 0 && this->Locations.Num() != 0)
    {
        RPCNetMulticastRemove(this->Locations.Last());
        --_nbToRemove;
    }
}

void USpacelInstancedMeshComponent::RPCNetMulticastUseForm_Implementation(EFormType _type, uint8 _ignoreLast)
{
    if (m_loaded.find(_type) != m_loaded.end())
    {
        initArrays(m_loaded[_type], _ignoreLast);
        BroadcastCount();
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

    initArrays(_locations, _ignoreLast);
    BroadcastCount();
    resetBuild();
}

void USpacelInstancedMeshComponent::clean()
{
    m_removedLocations.Append(this->Locations);
    Super::clean();
    BroadcastCount();
}

void USpacelInstancedMeshComponent::initArrays(TArray<FVector_NetQuantize> const& _in, uint8 _ignoreLast)
{
    auto locations = _in;

    auto lb_copyArray = [&locations](int32 _nb, TArray<FVector_NetQuantize>& _out)
    {
        _out.Empty();
        _out.Reserve(_nb);
        while (_nb != 0 && locations.Num())
        {
            _out.Add(locations[0]);
            locations.RemoveAt(0);
        }
    };

    lb_copyArray(_in.Num() - _ignoreLast, this->Locations);
    lb_copyArray(_ignoreLast, m_removedLocations);
}