// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "XmlInstancedStaticMeshComponent.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UXmlInstancedStaticMeshComponent : public UInstancedStaticMeshComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void Read();

	UFUNCTION(BlueprintCallable)
	void Export() const;
	
protected:
	UPROPERTY(Category = "Param", EditAnywhere, BlueprintReadWrite)
	FString Path {};
};
