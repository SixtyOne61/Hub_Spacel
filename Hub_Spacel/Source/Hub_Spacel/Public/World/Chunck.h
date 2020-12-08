// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/DestroyActor.h"
#include "Chunck.generated.h"

UCLASS()
class HUB_SPACEL_API AChunck : public ADestroyActor
{
	GENERATED_BODY()

	friend class AWorldManager;

public:
	// Sets default values for this actor's properties
	AChunck();

	/* override */
	virtual void Tick(float _deltaTime) override;
	virtual void dmg(FHitResult const& _info) override;

private:
	/* init actor, call by World Manager, most of time on editor, we keep  */
	bool init(FVector2D const& _bornX, FVector2D const& _bornY, FVector2D const& _bornZ, int32 _cubeSize);

	/* generate all static mesh component instance */
	bool generateChunck();

	/* get noise value */
	float getNoise(FVector const& _location) const;

	UFUNCTION()
	void OnComponentHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Settings")
	FVector2D BornX {};
	UPROPERTY(VisibleAnywhere, Category = "Settings")
	FVector2D BornY {};
	UPROPERTY(VisibleAnywhere, Category = "Settings")
	FVector2D BornZ {};
	UPROPERTY(VisibleAnywhere, Category = "Settings")
	int32 CubeSize {};

	UPROPERTY(EditAnywhere, Category = "Settings")
	class UStaticMesh* VoxelStaticMesh { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Component")
	class UInstancedStaticMeshComponent* Voxels { nullptr };

	TMap<int32, int16> m_dmg {};
};
