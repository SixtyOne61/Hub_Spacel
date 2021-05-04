// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/DestroyActor.h"
#include "NinePackActor.generated.h"

UCLASS()
class HUB_SPACEL_API ANinePackActor : public ADestroyActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANinePackActor();

protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;
	void applyHit(TArray<int32>& _instance) override;

private:
	UFUNCTION()
	void OnRep_RemoveInstance();

	UFUNCTION()
	void OnComponentHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit);

public:
	UPROPERTY(Replicated)
	bool R_IsBoost { false };

protected:
	UPROPERTY(VisibleAnywhere, Category = "Component")
	class UInstancedStaticMeshComponent* Voxels { nullptr };

	UPROPERTY(ReplicatedUsing = "OnRep_RemoveInstance")
	TArray<int32> RU_RemoveIndex{};

	// only use on client
	int m_countRemovedIndex{};
};
