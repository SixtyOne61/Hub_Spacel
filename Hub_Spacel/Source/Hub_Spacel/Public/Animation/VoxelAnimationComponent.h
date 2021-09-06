// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enum/SpacelEnum.h"
#include "Curves/CurveFloat.h"
#include "VoxelAnimationComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Animation), meta = (BlueprintSpawnableComponent))
class HUB_SPACEL_API UVoxelAnimationComponent : public UActorComponent
{
	GENERATED_BODY()
	
protected:
	struct SAnim
	{
		EAnimationCurveType m_type{};
		bool m_isUniform{ false };
		float m_duration{ 0.0f };
		float m_timer{ 0.0f };
		TArray<FVector_NetQuantize>* m_array{ nullptr };
		FVector m_delta{};
	};

public:
	UVoxelAnimationComponent();

	void start(TArray<FVector_NetQuantize> * _toAnim, EAnimationCurveType _type, bool _isUniform, FVector const& _delta, float _duration);

	void TickComponent(float _deltaSeconde, enum ELevelTick _tickType, FActorComponentTickFunction* _thisTickFunction) override;

public:
	UPROPERTY(Category = "Animation", BlueprintReadOnly, EditAnywhere)
	class UCurveAnimationDataAsset* AnimationDataAsset { nullptr };

private:
	void animUniform(SAnim & _anim, float _deltaSeconde);
	void animUnUniform(SAnim & _anim, float _deltaSeconde);

protected:
	TArray<SAnim> m_anims;
};
