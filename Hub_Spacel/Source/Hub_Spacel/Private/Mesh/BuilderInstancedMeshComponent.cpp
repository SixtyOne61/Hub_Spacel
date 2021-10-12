// Fill out your copyright notice in the Description page of Project Settings.


#include "BuilderInstancedMeshComponent.h"
#include "Util/SimplyXml.h"

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
    SimplyXml::FContainer<FVector_NetQuantize> locationInformation { "Location" };
    locationInformation.Values = this->Locations;

    SimplyXml::FWriter writer { FPaths::ProjectDir() + this->Path };
    writer.write(locationInformation);
}