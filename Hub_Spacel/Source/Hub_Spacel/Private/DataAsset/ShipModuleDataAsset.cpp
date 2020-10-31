// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipModuleDataAsset.h"
#include "ProceduralModuleDataAsset.h"

UProceduralModuleDataAsset const* UShipModuleDataAsset::GetModule(uint8 const& _flag)
{
    for (auto const* moduleDataAsset : this->Modules)
    {
        if (moduleDataAsset == nullptr)
        {
            continue;
        }

        if ((moduleDataAsset->UniqueModuleType & _flag) == _flag)
        {
            return moduleDataAsset;
        }
    }

    return nullptr;
}