// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Mesh/XmlInstancedStaticMeshComponent.h"
#include "BuilderInstancedMeshComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = Mesh, meta = (BlueprintSpawnableComponent), Blueprintable)
class HUB_SPACEL_API UBuilderInstancedMeshComponent : public UXmlInstancedStaticMeshComponent
{
	GENERATED_UCLASS_BODY()

	UBuilderInstancedMeshComponent(FVTableHelper& _helper);
	virtual ~UBuilderInstancedMeshComponent();
	
public:
	UFUNCTION(BlueprintCallable, Category = "Components|Xml")
	void Export() const;

public:
	UPROPERTY(Category = "Param", EditAnywhere, BlueprintReadWrite)
	int MaxVoxel {};
};
