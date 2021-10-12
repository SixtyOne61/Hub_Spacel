// Fill out your copyright notice in the Description page of Project Settings.


#include "BuilderInstancedMeshComponent.h"

UBuilderInstancedMeshComponent::UBuilderInstancedMeshComponent(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}


UBuilderInstancedMeshComponent::UBuilderInstancedMeshComponent(FVTableHelper& _helper)
    : Super(_helper)
{
}

UBuilderInstancedMeshComponent::~UBuilderInstancedMeshComponent()
{
}

void UBuilderInstancedMeshComponent::Export() const
{
    int32 instanceCount = this->GetInstanceCount();

    TArray<FVector> transforms; transforms.Reserve(instanceCount);
    while (--instanceCount >= 0)
    {
        FTransform out{};
        this->GetInstanceTransform(instanceCount, out);
        transforms.Add(out.GetLocation());
    }
}