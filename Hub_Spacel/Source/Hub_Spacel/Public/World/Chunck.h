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
	void applyHit(TArray<int32>& _instance) override;

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
	int R_ChunckSize {};

	UPROPERTY(Replicated)
	int16 R_CubeSize {};

	// TO DO : optim remove this, we can just have an rpc with index, and save remove index on server side.
	// on start, ask server (find how ask / reply to a specific client) and server reply with full remove index for reconnection
	UPROPERTY(ReplicatedUsing = "OnRep_RemoveInstance")
	TArray<int16> RU_RemoveIndex {};

	// only use on client
	int m_countRemovedIndex {};
};
