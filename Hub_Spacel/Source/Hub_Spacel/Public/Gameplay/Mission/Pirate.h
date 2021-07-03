// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/Mission/MissionActor.h"
#include "Pirate.generated.h"

UCLASS()
class HUB_SPACEL_API APirate : public AMissionActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APirate();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void BuildShip();

	UFUNCTION()
	void OnVoxelsHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit);

	UFUNCTION()
	void OnRedCubeHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit);

	UFUNCTION(Reliable, NetMulticast)
	void RPCNetMulticastHit(int32 _index);

protected:
	UPROPERTY(EditAnywhere)
	class UPirateDataAsset* DataAsset { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Component")
	class UInstancedStaticMeshComponent* Voxels { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Component")
	class UStaticMeshComponent* RedCube { nullptr };
};
