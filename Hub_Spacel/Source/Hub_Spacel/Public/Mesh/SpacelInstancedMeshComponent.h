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
	void UseForm(EFormType _type);

protected:
	UFUNCTION(Reliable, NetMulticast, Category = "Components|Replication")
	void RPCNetMulticastUseForm(EFormType _type);

	UFUNCTION(Reliable, NetMulticast, Category = "Components|Replication")
	void RPCNetMulticastAddForm(EFormType _type, TArray<FVector_NetQuantize> const& _locations);

protected:
	UPROPERTY(Category = "Param", EditAnywhere, BlueprintReadWrite)
	TArray<class UFormDataAsset*> Forms;

	UPROPERTY(Category = "Setup", EditAnywhere, BlueprintReadWrite)
	bool UseBonus { false };

private:
	/* form already loaded and send to client */
	std::unordered_map<EFormType, TArray<FVector_NetQuantize>> m_loaded;
};
