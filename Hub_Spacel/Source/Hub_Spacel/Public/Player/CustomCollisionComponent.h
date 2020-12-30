// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/PlayerActorComponent.h"
#include "CustomCollisionComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HUB_SPACEL_API UCustomCollisionComponent : public UPlayerActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCustomCollisionComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	bool saveDestroyActor(TArray<FHitResult> & _items, TArray<FHitResult> const& _hits);
};
