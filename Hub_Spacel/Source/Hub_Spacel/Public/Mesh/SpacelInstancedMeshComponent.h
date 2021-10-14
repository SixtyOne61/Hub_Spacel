// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Mesh/NetworkInstancedMeshComponent.h"
#include "Enum/SpacelEnum.h"
#include <unordered_map>
#include "SpacelInstancedMeshComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = Mesh, meta = (BlueprintSpawnableComponent), Blueprintable)
class HUB_SPACEL_API USpacelInstancedMeshComponent : public UNetworkInstancedMeshComponent
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Components|Form")
	void UseForm(EFormType _type, bool _refresh);

	UFUNCTION(BlueprintCallable, Category = "Components|Repair")
	uint8 Repair(uint8 _nbRepair);

	UFUNCTION(BlueprintCallable, Category = "Components|Repair")
	void RemoveRandom(uint8 _nbToRemove);

	UFUNCTION(BlueprintCallable, Category = "Components|Setteur")
	void SetUseBonus(bool _use) { UseBonus = _use; }

	UFUNCTION(BlueprintCallable, Category = "Components|Getteur")
	int32 GetMax() const { return Locations.Num() + m_removedLocations.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Components|Getteur")
	int32 GetNbRemoved() const { return m_removedLocations.Num(); }

protected:
	UFUNCTION(Reliable, NetMulticast, Category = "Components|Replication")
	void RPCNetMulticastUseForm(EFormType _type, uint8 _ignoreLast);

	UFUNCTION(Reliable, NetMulticast, Category = "Components|Replication")
	void RPCNetMulticastAddForm(EFormType _type, TArray<FVector_NetQuantize> const& _locations, uint8 _ignoreLast);

protected:
	inline int Remove(FVector_NetQuantize const& _location) override 
	{
		m_removedLocations.Add(_location);
		return Super::Remove(_location);
	}

	inline void BroadcastCount() const override
	{
		OnUpdateCountDelegate.Broadcast(this->Locations, GetMax());
	}

	/* save location before clear */
	void clean() override;

	/* init location and m_removedLocations with _in*/
	void initArrays(TArray<FVector_NetQuantize> const& _in, uint8 _ignoreLast);

protected:
	UPROPERTY(Category = "Param", EditAnywhere, BlueprintReadWrite)
	TArray<class UFormDataAsset*> Forms;

	UPROPERTY(Category = "Setup", EditAnywhere, BlueprintReadWrite)
	bool UseBonus { false };

private:
	/* form already loaded and send to client */
	std::unordered_map<EFormType, TArray<FVector_NetQuantize>> m_loaded;

	/* remove location, only use server side */
	TArray<FVector_NetQuantize> m_removedLocations;
};
