// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/PlayerActorComponent.h"
#include "Util/Optional.h"
#include "FireComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HUB_SPACEL_API UFireComponent : public UPlayerActorComponent
{
	GENERATED_BODY()

	friend class AShipPawn;

public:	
	// Sets default values for this component's properties
	UFireComponent();

	// Called every frame
	void TickComponent(float _deltaTime, ELevelTick _tickType, FActorComponentTickFunction* _thisTickFunction) override;

private:
	void spawnBullet(FTransform const& _transform) const;

private:
	/* use only on server, say if we are in fire */
	Util::Optional<bool> m_isFire{ };

	/* current time between next bullet */
	float m_fireCountDown{ };

	/* fire point location */
	int32 m_fireIndex{ };

	/* current targeted actor */
	class AActor* m_target{ nullptr };
};
