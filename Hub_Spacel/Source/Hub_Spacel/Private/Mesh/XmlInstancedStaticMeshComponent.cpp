// Fill out your copyright notice in the Description page of Project Settings.


#include "XmlInstancedStaticMeshComponent.h"

void UXmlInstancedStaticMeshComponent::Read()
{

}

void UXmlInstancedStaticMeshComponent::Export() const
{
    int32 instanceCount = this->GetInstanceCount();

    TArray<FVector> transforms; transforms.Reserve(instanceCount);
    while (--instanceCount >= 0)
    {
        FTransform out {};
        this->GetInstanceTransform(instanceCount, out);
        transforms.Add(out.GetLocation());
    }
}