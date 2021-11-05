// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Mesh/SpacelInstancedMeshComponent.h"
#include "AnimatedSpacelMeshComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = Mesh, meta = (BlueprintSpawnableComponent), Blueprintable)
class HUB_SPACEL_API UAnimatedSpacelMeshComponent : public USpacelInstancedMeshComponent
{
	GENERATED_BODY()

public:
	UAnimatedSpacelMeshComponent();
	
protected:
	/* only workd on client side */
	void build() override;

	/* override for add save when we use a form */
	void populate(EFormType _type, uint8 _ignoreLast, bool _useBonus) override;

	// Called every frame
	void TickComponent(float _deltaTime, ELevelTick _tickType, FActorComponentTickFunction* _thisTickFunction) override;

	/* local build ship for animation */
	bool localBuild();

	/* for client, find this position in m_locationOnStart if we are in animation */
	int Remove(FVector_NetQuantize const& _location) override;

	/* we don't anim locations added 
	TO DO with can save removed location like m_locationOnStart
	and find the previous location of removed, and "repair" locationOnStart with it */
	inline void AddRange(TArray<FVector_NetQuantize> const& _locations) override
	{
		if (this->GetNetMode() != ENetMode::NM_DedicatedServer)
		{
			for (auto loc : _locations)
			{
				m_locationOnStart.Add(loc);
			}
		}
		Super::AddRange(_locations);
	}

	/* same as AddRange */
	inline void Add(FVector_NetQuantize const& _location) override
	{
		if (this->GetNetMode() != ENetMode::NM_DedicatedServer)
		{
			m_locationOnStart.Add(_location);
		}
		Super::Add(_location);
	}

private:
	/* when != 0.0f, we are animating */
	float m_timer { 0.0f };

	/* last instance vector */
	TArray<FVector_NetQuantize> m_locationOnStart {};

protected:
	UPROPERTY(Category = "Param", EditAnywhere, BlueprintReadWrite)
	float AnimationDuration { 0.35f };
};
