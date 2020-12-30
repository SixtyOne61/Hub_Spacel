// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerActorComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HUB_SPACEL_API UPlayerActorComponent : public UActorComponent
{
	GENERATED_BODY()

protected:
	// Called when the game starts
	void BeginPlay() override;

	bool initShipPawnOwner();

protected:
	TWeakObjectPtr<class AShipPawn> m_shipPawnOwner{};
};
