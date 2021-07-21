// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SpacelEnum.h"
#include "DataAsset/MissionDataAsset.h"
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

	UPROPERTY()
	FTimerHandle ShowDitactitialHandle {};

protected:
	void NativeConstruct() override;
	void NativeDestruct() override;
	void NativeTick(const FGeometry& _myGeometry, float _deltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Setup")
	void SetBackgroundTeamColor(FSlateColor const& _teamColor);

	UFUNCTION()
	void OnChangeState(EGameState _state);
	
	UFUNCTION(BlueprintImplementableEvent)
	void BP_EndGame();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_UnlockInput();

	UFUNCTION()
	void UpdateScore();

	UFUNCTION()
	void SetLatestEvent();

	UFUNCTION()
	void SetAverragePlayerLatency();

	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateMatiere(int32 _value, FString const& _deltaStr);

	UFUNCTION()
	void OnUpdateCountProtection(int32 _value, int32 _max);

	UFUNCTION()
	void OnUpdateCountSupport(int32 _value, int32 _max);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdatePercent(class UProgressBar* _progressBar, class UTextBlock* _text, int32 _value, int32 _max);

	UFUNCTION()
	void RedLight();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_RedLight(int32 _level);

	UFUNCTION()
	void OnShowScore(bool _show);

	UFUNCTION()
	void OnAddEffect(EEffect _type);

	UFUNCTION()
	void OnRemoveEffect(EEffect _type);

	UFUNCTION(BlueprintImplementableEvent)
	void OnRemoveEffectFx(class UTexture2D* _texture);

	UFUNCTION(BlueprintImplementableEvent)
	void OnSendInfoPlayer(FString const& _text);

	UFUNCTION(BlueprintImplementableEvent)
	void OnRespawn();

	UFUNCTION()
	void OnStartMission(EMission _type);

	UFUNCTION()
	void OnStartMissionTwoParam(EMission _type, FName const& _team);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_ShowMissionPanel();

	UFUNCTION()
	void OnEndMission(EMission _type);

	UFUNCTION(BlueprintImplementableEvent)
	void HideMissionPanel();

	void addSkill(class SkillCountDown * _skill);

	UFUNCTION()
	void RegisterPlayerState();

protected:
	UPROPERTY(EditAnywhere)
	class UEffectDataAsset* EffectDataAsset { nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UKeyDataAsset* KeyDataAsset{ nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UTeamColorDataAsset* TeamColorDataAsset{ nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UMissionDataAsset* MissionDataAsset { nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UFlyingGameModeDataAsset* GameModeDataAsset{ nullptr };

	UPROPERTY()
	FTimerHandle RedLightAnimationHandle {};

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UEffectWidget> EffectWidgetClass { nullptr };

	UPROPERTY()
	class UTextBlock* EventTextBlock { nullptr };

	UPROPERTY()
	class UTextBlock* TimerTextBlock{ nullptr };

	UPROPERTY()
	class UTextBlock* PingTextBlock { nullptr };

	UPROPERTY()
	class UProgressBar* ProtectionProgressBar { nullptr };

	UPROPERTY()
	class UTextBlock* ProtectionTextBlock{ nullptr };

	UPROPERTY()
	class UProgressBar* SupportProgressBar { nullptr };

	UPROPERTY()
	class UTextBlock* SupportTextBlock { nullptr };

	UPROPERTY()
	class UHorizontalBox* SkillBarHorizontalBox { nullptr };

	UPROPERTY()
	class UHorizontalBox* EffectBarHorizontalBox { nullptr };

	UPROPERTY()
	TArray<class UAllyWidget*> AllyWidgets { };

	UPROPERTY()
	TArray<class UBorder*> TeamScoreColorWidgets { };

	UPROPERTY()
	TArray<class UTextBlock*> TeamScoreWidgets{ };

	FString Team {};

	int32 m_nextTipsId { 0 };
	int8 m_currentIdRedLight { 0 };

	TSet<EMission> m_currentMission {};
};
