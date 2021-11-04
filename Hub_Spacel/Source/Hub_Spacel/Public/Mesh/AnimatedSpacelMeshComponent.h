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

private:
	/* when != 0.0f, we are animating */
	float m_timer { 0.0f };

	/* last instance vector */
	TArray<FVector_NetQuantize> m_locationOnStart {};

protected:
	UPROPERTY(Category = "Param", EditAnywhere, BlueprintReadWrite)
	float AnimationDuration { 0.35f };
};
