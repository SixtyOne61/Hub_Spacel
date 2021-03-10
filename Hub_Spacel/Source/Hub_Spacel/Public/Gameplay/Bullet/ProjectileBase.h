// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/DestroyActor.h"
#include "ProjectileBase.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API AProjectileBase : public ADestroyActor
{
	GENERATED_BODY()
	
public:
	AProjectileBase();

protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;

	virtual bool OnHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit);

	FString && getLocalTeam() const;

public:
	UPROPERTY(EditAnywhere, Category = "Default")
	class UProjectileMovementComponent* ProjectileMovementComponent{ nullptr };

protected:
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* FireFx{ nullptr };
};
