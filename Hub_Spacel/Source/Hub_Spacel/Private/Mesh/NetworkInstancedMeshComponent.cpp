// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkInstancedMeshComponent.h"

void UNetworkInstancedMeshComponent::RPCNetMulticastAdd_Implementation(FVector_NetQuantize const& _location)
{
    this->Add(_location);
}

void UNetworkInstancedMeshComponent::RPCNetMulticastRemove_Implementation(FVector_NetQuantize const& _location)
{
    this->Remove(_location);
}

void UNetworkInstancedMeshComponent::resetBuild()
{
    this->RPCNetMulticastResetBuild();
}

void UNetworkInstancedMeshComponent::RPCNetMulticastResetBuild_Implementation()
{
    this->ClearInstances();
    for (auto loc : this->Locations)
    {
        this->AddInstance(FTransform{ loc });
    }
}