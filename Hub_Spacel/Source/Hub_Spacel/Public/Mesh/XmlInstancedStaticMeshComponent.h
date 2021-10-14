// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "XmlInstancedStaticMeshComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = Mesh, meta = (BlueprintSpawnableComponent), Blueprintable)
class HUB_SPACEL_API UXmlInstancedStaticMeshComponent : public UInstancedStaticMeshComponent
{
	GENERATED_UCLASS_BODY()

	UXmlInstancedStaticMeshComponent(FVTableHelper& _helper);
	virtual ~UXmlInstancedStaticMeshComponent();

public:
	UFUNCTION(BlueprintCallable, Category = "Components|Xml")
	void Read(bool _useBonus);

	UFUNCTION(BlueprintCallable, Category = "Components|Action")
	virtual void Add(FVector_NetQuantize const& _location);

	UFUNCTION(BlueprintCallable, Category = "Components|Action")
	virtual int Remove(FVector_NetQuantize const& _location);

	UFUNCTION(BlueprintCallable, Category = "Components|Getteur")
	inline int32 GetNum() const { return this->Locations.Num(); }

	UFUNCTION(BlueprintCallable, Category = "Components|Getteur")
	inline TArray<FVector_NetQuantize> const& GetLocations() const { return this->Locations; }
	
protected:
	/* take Locations and add it to instance static mesh */
	virtual void resetBuild();

protected:
	UPROPERTY(Category = "Param", EditAnywhere, BlueprintReadWrite)
	FString Path {};

	UPROPERTY(Category = "Array", EditAnywhere, BlueprintReadWrite)
	TArray<FVector_NetQuantize> Locations {};

protected:
	TArray<FVector_NetQuantize> m_baseLocations {};
	TArray<FVector_NetQuantize> m_bonusLocations{};
};
