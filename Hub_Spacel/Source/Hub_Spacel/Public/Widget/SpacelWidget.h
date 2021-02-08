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
	FTimerHandle SetTeammateCountHandle {};

	UPROPERTY()
	FTimerHandle SetLatestEventHandle {};

	UPROPERTY()
	FTimerHandle SetAverragePlayerLatencyHandle {};

	UPROPERTY()
	FTimerHandle SetSpeedHandle {};

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void StartGame();

	UFUNCTION()
	void SetTeammateCount();

	UFUNCTION()
	void SetLatestEvent();

	UFUNCTION()
	void SetAverragePlayerLatency();

	UFUNCTION()
	void SetSpeed();

	UFUNCTION()
	void OnToggleRepair(bool _on);

	UFUNCTION()
	void OnUpdateMatiere(int32 _value);

	UFUNCTION()
	void OnChangeStateEscapeMode(EEscapeMode _state);

	UFUNCTION()
	void OnUpdateCountProtection(int32 _value, int32 _max);

	UFUNCTION()
	void OnUpdateCountSupport(int32 _value, int32 _max);

	void updateCount(class UTextBlock* _textBlock, FString && _str);

private:
	UPROPERTY()
	class UTextBlock* TeamNameTextBlock { nullptr };

	UPROPERTY()
	class UTextBlock* MatiereTextBlock { nullptr };

	UPROPERTY()
	class UTextBlock* ProtectionTextBlock{ nullptr };

	UPROPERTY()
	class UTextBlock* SupportTextBlock{ nullptr };

	UPROPERTY()
	class UTextBlock* TeammateCountTextBlock { nullptr };

	UPROPERTY()
	class UTextBlock* EventTextBlock { nullptr };

	UPROPERTY()
	class UTextBlock* PingTextBlock { nullptr };

	UPROPERTY()
	class UTextBlock* SpeedTextBlock { nullptr };

	UPROPERTY()
	class URepairUserWidget* ToggleRepairGridPanel { nullptr };

	UPROPERTY()
	class UImage* EscapeModeImage { nullptr };
};
