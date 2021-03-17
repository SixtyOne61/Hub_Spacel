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
	void OnUpdateCountProtection(int32 _value, int32 _max);

	UFUNCTION()
	void OnUpdateCountSupport(int32 _value, int32 _max);

	void updatePercent(class UProgressBar* _progressBar, float _percent);

	UFUNCTION()
	void OnShowScore(bool _show);

	UFUNCTION()
	void OnAddEffect(EEffect _type);

	UFUNCTION()
	void OnRemoveEffect(EEffect _type);

	template<class T>
	void setVisibility(T* _widget, bool _show)
	{
		if (_widget == nullptr) return;

		if (_show)
		{
			_widget->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			_widget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

protected:
	UPROPERTY(EditAnywhere)
	class UEffectDataAsset* EffectDataAsset { nullptr };

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class USkillWidget> SkillWidgetClass { nullptr };

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UEffectWidget> EffectWidgetClass { nullptr };

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
	class UHorizontalBox* SkillBarHorizontalBox { nullptr };

	UPROPERTY()
	class UHorizontalBox* EffectBarHorizontalBox { nullptr };

	UPROPERTY()
	TArray<class UAllyWidget*> AllyWidgets { };

	UPROPERTY()
	class UUserWidget* ScoreWidget { nullptr };
};
