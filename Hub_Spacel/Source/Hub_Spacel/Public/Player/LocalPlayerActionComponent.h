// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/PlayerActorComponent.h"
#include "Gameplay/Metric/LocalMetric.h"
#include "Enum/SpacelEnum.h"
#include <memory>
#include "LocalPlayerActionComponent.generated.h"

/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HUB_SPACEL_API ULocalPlayerActionComponent : public UPlayerActorComponent
{
	GENERATED_BODY()
	
public:
	ULocalPlayerActionComponent();
	~ULocalPlayerActionComponent();

	void BeginPlay() override;
	void TickComponent(float _deltaTime, ELevelTick _tickType, FActorComponentTickFunction* _thisTickFunction) override;

	template<class ... Ts>
	void useMetric(EMetric _type, Ts ... _args)
	{
		switch (_type)
		{
		case EMetric::Precision:
			createPrecisionData(std::forward<Ts>(_args)...);
		break;
		}
	}

private:
	UFUNCTION()
	void OnUpdateTeam(FString const& _team);

	UFUNCTION()
	void AddEffect(EEffect _effect);

	UFUNCTION()
	void RemoveEffect(EEffect _effect);

	UFUNCTION()
	void OnScored(EScoreType _type, int32 _value);

private:
	void createPrecisionData(bool _success);

private:
	class UMaterialInstanceDynamic* m_speedLineMaterial { nullptr };

	std::unique_ptr<LocalMetric> m_metric { nullptr };
};
