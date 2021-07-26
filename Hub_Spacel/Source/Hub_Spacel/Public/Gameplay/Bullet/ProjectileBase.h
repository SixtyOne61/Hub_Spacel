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

	/* override */
	void applyHit(TArray<int32>& _instance) override;

protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;

	virtual bool OnHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit);

	FString getLocalTeam() const;

	UFUNCTION()
	void OnComponentHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit);

public:
	UPROPERTY(EditAnywhere, Category = "Collision")
	class USphereComponent* ProjectileCollisionComponent{ nullptr };

	UPROPERTY(Replicated)
	FName R_Team {};

	UPROPERTY(Replicated)
	int32 PlayerIdOwner {};
};
