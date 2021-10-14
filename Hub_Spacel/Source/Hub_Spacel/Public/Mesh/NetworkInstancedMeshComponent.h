// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Mesh/XmlInstancedStaticMeshComponent.h"
#include "NetworkInstancedMeshComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpdateCount, TArray<FVector_NetQuantize> const&, _locations, int32, _max);

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

	UFUNCTION(Reliable, NetMulticast, Category = "Components|Replication")
	void RPCNetMulticastClean();

protected:
	UFUNCTION(Reliable, NetMulticast, Category = "Components|Replication")
	void RPCNetMulticastResetBuild();

	UFUNCTION(BlueprintCallable, Category = "Components|Event")
	inline virtual void BroadcastCount() const
	{
		OnUpdateCountDelegate.Broadcast(this->Locations, this->Locations.Num());
	}

protected:
	/* hide for this component and children */
	inline void Add(FVector_NetQuantize const& _location) override
	{
		Super::Add(_location);
		BroadcastCount();
	}

	inline int Remove(FVector_NetQuantize const& _location) override
	{
		auto ret = Super::Remove(_location);
		BroadcastCount();
		return ret;
	}

	/* call rpc to rebuild on all clients and server */
	void resetBuild() override;

	/* clean instance can be override in children */
	virtual void clean();

public:
	UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
	FOnUpdateCount OnUpdateCountDelegate {};
};
