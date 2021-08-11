// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/PlayerActorComponent.h"
#include "Gameplay/Metric/LocalMetric.h"
#include "Enum/SpacelEnum.h"
#include "MetricComponent.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UMetricComponent : public UPlayerActorComponent
{
	GENERATED_BODY()

public:
	void BeginPlay() override;

	void createPrecisionData(bool _success);
	void createFogData();
	void createKillData();
	void createEmpPointData(uint16 _value);
	void createTankPointData(uint16 _value);
	void createMatiereWinData(uint16 _value);
	void createMatiereRepair(uint16 _value);
	void createTotalPointData(uint16 _value);

private:
	UFUNCTION()
	void OnScored(EScoreType _type, int32 _value);

	UFUNCTION()
	void AddEffect(EEffect _effect);

	UFUNCTION()
	void OnChangeState(EGameState _state);

	UFUNCTION(Reliable, NetMulticast)
	void RPCNetMulticastSendData(uint8 _precision, uint8 _nbKill, uint16 _totalScore);
	
public:
	UPROPERTY()
	uint8 Precision { 0 };

	UPROPERTY()
	uint8 NbKill { 0 };

	UPROPERTY()
	uint16 TotalScore { 0 };

	UPROPERTY()
	bool HasInit { false };

private:
	std::unique_ptr<LocalMetric> m_metric{ nullptr };
};
