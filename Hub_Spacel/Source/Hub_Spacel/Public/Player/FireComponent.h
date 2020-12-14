// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Util/Optional.h"
#include "FireComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HUB_SPACEL_API UFireComponent : public UActorComponent
{
	GENERATED_BODY()

	friend class AShipPawn;

public:	
	// Sets default values for this component's properties
	UFireComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float _deltaTime, ELevelTick _tickType, FActorComponentTickFunction* _thisTickFunction) override;

private:
	TSharedPtr<class AShipPawn> m_shipPawnOwner {};

	/* use only on server, say if we are in fire */
	Util::Optional<bool> m_isFire{ };

	/* current time between next bullet */
	float m_fireCountDown{ };

	/* fire point location */
	int32 m_fireIndex{ };

	/* current targeted actor */
	class AActor* m_target{ nullptr };
};
