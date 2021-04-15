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

	template<class T>
	inline T* get() { return Cast<T>(get()); }

	template<class T>
	inline T* get() const { return Cast<T>(get()); }

	inline class ACommonPawn* get() { return m_shipPawnOwner.Get(); }
	inline class ACommonPawn* get() const { return m_shipPawnOwner.Get(); }

private:
	TWeakObjectPtr<class ACommonPawn> m_shipPawnOwner{};
};
