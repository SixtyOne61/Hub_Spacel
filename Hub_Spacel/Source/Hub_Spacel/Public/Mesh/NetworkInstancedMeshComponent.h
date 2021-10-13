// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Mesh/XmlInstancedStaticMeshComponent.h"
#include "NetworkInstancedMeshComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = Mesh, meta = (BlueprintSpawnableComponent), Blueprintable)
class HUB_SPACEL_API UNetworkInstancedMeshComponent : public UXmlInstancedStaticMeshComponent
{
	GENERATED_BODY()
	
public:
	UFUNCTION(Reliable, NetMulticast, Category = "Components|Replication")
	void RPCNetMulticastAdd(FVector_NetQuantize const& _location);

	UFUNCTION(Reliable, NetMulticast, Category = "Components|Replication")
	void RPCNetMulticastRemove(FVector_NetQuantize const& _location);

protected:
	UFUNCTION(Reliable, NetMulticast, Category = "Components|Replication")
	void RPCNetMulticastResetBuild();

protected:
	/* hide for this component and children */
	void Add(FVector_NetQuantize const& _location) override { Super::Add(_location); }
	int Remove(FVector_NetQuantize const& _location) override { return Super::Remove(_location); }

	void resetBuild() override;
};
