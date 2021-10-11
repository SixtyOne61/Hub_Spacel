// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "XmlInstancedStaticMeshComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = Rendering, meta = (BlueprintSpawnableComponent), Blueprintable)
class HUB_SPACEL_API UXmlInstancedStaticMeshComponent : public UInstancedStaticMeshComponent
{
	GENERATED_UCLASS_BODY()

	UXmlInstancedStaticMeshComponent(FVTableHelper& _helper);
	virtual ~UXmlInstancedStaticMeshComponent();

public:
	UFUNCTION(BlueprintCallable, Category = "Components|Xml")
	void Read();

	UFUNCTION(BlueprintCallable, Category = "Components|Xml")
	void Export() const;

	UFUNCTION(BlueprintCallable, Category = "Components|Action")
	void Add(FVector_NetQuantize const& _location);

	UFUNCTION(BlueprintCallable, Category = "Components|Action")
	void Remove(FVector_NetQuantize const& _location);
	
protected:
	UPROPERTY(Category = "Param", EditAnywhere, BlueprintReadWrite)
	bool IsOffline {};

	UPROPERTY(Category = "Param", EditAnywhere, BlueprintReadWrite)
	FString Path {};

	UPROPERTY(Category = "Array", EditAnywhere, BlueprintReadWrite)
	TArray<FVector_NetQuantize> Locations {};
};
