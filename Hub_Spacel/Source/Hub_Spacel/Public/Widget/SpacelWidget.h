// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SpacelEnum.h"
#include "SpacelWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API USpacelWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	FTimerHandle SetLatestEventHandle {};

	UPROPERTY()
	FTimerHandle SetAverragePlayerLatencyHandle {};

	UPROPERTY()
	FTimerHandle SetSpeedHandle {};

protected:
	void NativeConstruct() override;
	void NativeDestruct() override;
	void NativeTick(const FGeometry& _myGeometry, float _deltaTime) override;

private:
	UFUNCTION()
	void StartGame();

	UFUNCTION()
	void UpdateScore();

	UFUNCTION()
	void SetLatestEvent();

	UFUNCTION()
	void SetAverragePlayerLatency();

	UFUNCTION()
	void OnUpdateMatiere(int32 _value);

	UFUNCTION()
	void OnChangeStateEscapeMode(ECountDown _state);

	UFUNCTION()
	void OnUpdateCountProtection(int32 _value, int32 _max);

	UFUNCTION()
	void OnUpdateCountSupport(int32 _value, int32 _max);

	void updatePercent(class UProgressBar* _progressBar, float _percent);

	UFUNCTION()
	void OnShowScore(bool _show);

private:
	UPROPERTY()
	class UTextBlock* MatiereTextBlock { nullptr };

	UPROPERTY()
	class UTextBlock* EventTextBlock { nullptr };

	UPROPERTY()
	class UTextBlock* PingTextBlock { nullptr };

	UPROPERTY()
	class UProgressBar* ProtectionProgressBar { nullptr };

	UPROPERTY()
	class UProgressBar* SupportProgressBar { nullptr };

	UPROPERTY()
	class UProgressBar* EscapeModeProgressBar { nullptr };

	UPROPERTY()
	TArray<class UAllyWidget*> AllyWidgets { };

	UPROPERTY()
	class UUserWidget* ScoreWidget { nullptr };

	ECountDown m_escapeMode { ECountDown::Available };
	float m_duration { 0.0f };
};
