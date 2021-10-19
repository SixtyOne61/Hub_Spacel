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

            if(m_loaded.find(_type) != m_loaded.end())
            {
                RPCNetMulticastUseForm(_type, m_removedLocations.Num(), this->UseBonus);
            }
            else
            {
                // read xml
                this->Path = dataAsset->Path;
                Read();
                RPCNetMulticastAddForm(_type, m_baseLocations, m_bonusLocations, m_removedLocations.Num(), this->UseBonus);
            }
            return;
        }
    }

    _type != EFormType::Base ? UseForm(EFormType::Base, _refresh) : ensure(false);
}

uint8 USpacelInstancedMeshComponent::Repair(uint8 _nbRepair)
{
    TArray<FVector_NetQuantize> locations;
    int index { m_removedLocations.Num() - 1 };
    while (_nbRepair > 0 && index >= 0)
    {
        locations.Add(m_removedLocations[index]);
        --_nbRepair;
        -- index;
    }

    if (locations.Num() != 0)
    {
        RPCNetMulticastAddRange(locations);
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

void USpacelInstancedMeshComponent::RPCNetMulticastUseForm_Implementation(EFormType _type, uint8 _ignoreLast, bool _useBonus)
{
    if (m_loaded.find(_type) != m_loaded.end())
    {
        m_baseLocations = std::get<0>(m_loaded[_type]);
        m_bonusLocations = std::get<1>(m_loaded[_type]);

        populate(_type, _ignoreLast, _useBonus);
    }
    else
    {
        // TO DO : fallback
        UE_LOG(LogTemp, Warning, TEXT("Ship not loaded"));
    }
}

void USpacelInstancedMeshComponent::RPCNetMulticastAddForm_Implementation(EFormType _type, TArray<FVector_NetQuantize> const& _baseLocations, TArray<FVector_NetQuantize> const& _bonusLocations, uint8 _ignoreLast, bool _useBonus)
{
    m_baseLocations = _baseLocations;
    m_bonusLocations = _bonusLocations;

    m_loaded.insert({ _type , std::make_tuple(m_baseLocations, m_bonusLocations) });

    populate(_type, _ignoreLast, _useBonus);
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

void USpacelInstancedMeshComponent::initMesh(EFormType _type)
{
    for (auto* dataAsset : this->Forms)
    {
        if (dataAsset != nullptr && dataAsset->Type == _type)
        {
            this->SetStaticMesh(dataAsset->StaticMesh);
            this->SetEnableGravity(false);
        }
    }
}

void USpacelInstancedMeshComponent::populate(EFormType _type, uint8 _ignoreLast, bool _useBonus)
{
    initMesh(_type);
    InitLocations(_useBonus);
    initArrays(this->Locations, _ignoreLast);
    BroadcastCount();
    resetBuild();
}