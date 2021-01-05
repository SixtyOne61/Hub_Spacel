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
	void dmg(FHitResult const& _info) override;
	void applyDmg() override;

private:
	/* init actor, call by World Manager, most of time on editor, we keep  */
	void init(int _chunckSize, int32 _cubeSize);

	/* generate all static mesh component instance */
	void generateChunck(bool _isServer);

	/* get noise value */
	float getNoise(FVector const& _location, FVector2D const& _bornX, FVector2D const& _bornY, FVector2D const& _bornZ) const;

	UFUNCTION()
	void OnComponentHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnRep_RemoveInstance();

private:
	UPROPERTY(EditAnywhere, Category = "Settings")
	class UStaticMesh* VoxelStaticMesh { nullptr };

	UPROPERTY(EditAnywhere, Category = "Settings")
	TSubclassOf<class AFogActor> FogClass { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Component")
	class UInstancedStaticMeshComponent* Voxels { nullptr };

	UPROPERTY(Replicated)
	int32 R_RandomSeed { 0 };

	UPROPERTY(Replicated)
	int R_ChunckSize {};

	UPROPERTY(Replicated)
	int32 R_CubeSize {};

	UPROPERTY(ReplicatedUsing = "OnRep_RemoveInstance")
	TArray<int32> RU_RemoveIndex {};

	// only use on client
	int m_countRemovedIndex {};

	// only use on server
	TMap<int32, int16> m_dmg {};
};
