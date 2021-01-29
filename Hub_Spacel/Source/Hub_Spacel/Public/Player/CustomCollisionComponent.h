// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/PlayerActorComponent.h"
#include "CollisionShape.h"
#include "CustomCollisionComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HUB_SPACEL_API UCustomCollisionComponent : public UPlayerActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCustomCollisionComponent();

	void BeginPlay() override;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// TO DO : find why laser bullet trigger her component hit
	void hit(class UPrimitiveComponent* _comp, int32 _index);

private:
	/* return true if we find something and fill param */
	bool sweepByProfile(TArray<FHitResult> & _outHit, FVector const& _worldLocation, FName const& _profile, FCollisionShape const& _shape, TArray<FName> const& _ignoreTags, bool _drawDebug = false) const;
	bool sweepByProfile(TArray<FHitResult>& _outHit, FVector const& _worldLocation, FName const& _profile, FCollisionShape const& _shape, bool _drawDebug = false) const;

	/* manage if we hit matiere */
	void hitMatiere(TArray<FHitResult>& _items) const;

	/* create collision shape */
	template<class T>
	FCollisionShape createCollisionShapeWithLocalBounds(T const* _obj, FVector const& _scale) const
	{
		FVector min{}, max{};
		_obj->GetLocalBounds(min, max);
		FVector box = (max - min) * _scale;
		return FCollisionShape::MakeBox(box / 2.0f);
	}

	/* dispatch to hit actor */
	void dispatch(TArray<FHitResult> const& _items) const;

	/* check for each instance, can spawn matiere */
	bool sweepForInstancedStaticMesh(class UInstancedStaticMeshComponent*& _mesh, TArray<FVector>& _replicated, TArray<FVector>& _removeReplicated, FVector const& _scale, FName const& _profile);

private:
	/* matiere manager */
	TWeakObjectPtr<class AMatiereManager> m_matiereManager{};
};
