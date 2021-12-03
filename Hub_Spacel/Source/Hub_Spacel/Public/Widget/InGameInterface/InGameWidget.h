// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/SpacelEnum.h"
#include "DataAsset/MissionDataAsset.h"
#include "InGameWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UInGameWidget : public UUserWidget
{
	GENERATED_BODY()
		
protected:
	void NativeConstruct() override;
	void NativeDestruct() override;
	void NativeTick(const FGeometry& _myGeometry, float _deltaSeconde) override;

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetupOutline(FSlateColor _color);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI|Carrousel")
	void BP_AddSkillToChoose(ESkill _id, ESkillType _type, UTexture2D * _icon, FSlateColor const& _backgroundColor, FString const& _title, FString const& _desc, FString const& _verboseEffect, class USkillItemWidget*& _out);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_UpdateTimer(FString const& _str);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_GoAnimation();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetupTeam(class UTexture2D* _logo, FSlateColor _color, FString const& _team);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_UpdateScore(FString const& _team, int32 _value, bool _isBest);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetupInGame();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnUpdateMatiere(int32 _value, FString const& _deltaStr);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnSendInfoPlayer(FString const& _str);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnUpdateCountProtection(int32 _value, int32 _max);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnUpdateCountSupport(int32 _value, int32 _max);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnAddEffect(EEffect _type);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnRemoveEffect(EEffect _type);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_StartMission(FMission const& _mission);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnEndMission(FMission const& _mission);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnResetTimerMission(FMission const& _mission);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnScored(EScoreType _type, FString const& _value, FVector2D const& _position);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_CreateKillField(FString const& _killerName, FSlateColor const& _killerColor, FString const& _killedName, FSlateColor const& _killedColor);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_CreateAllyWidget(ASpacelPlayerState* _playerState);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_EndGame();

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnHoverSkill(FString const& _title, FString const& _desc, FSlateColor const& _color);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_OnChooseSkill(FString const& _verboseEffect, ESkillType _type, UTexture2D* _icon, FSlateColor const& _color);

private:
	UFUNCTION()
	void OnChangeState(EGameState _state);

	UFUNCTION()
	void WaitPlayerState();

	UFUNCTION()
	void OnStartMission(EMission _type);

	UFUNCTION()
	void OnStartMissionTwoParam(EMission _type, FName const& _team, FName const& _targetTeam);

	UFUNCTION()
	void OnEndMission(EMission _type, bool _succeed, FName _succeedForTeam);

	UFUNCTION()
	void InitMissionArrow(FName const& _tag);

	UFUNCTION()
	void OnResetTimerMission(EMission _type);

	UFUNCTION()
	void OnScored(EScoreType _type, int32 _value);

	UFUNCTION()
	void OnKill(int32 _killer, int32 _killed);

private:
	/* spawn method */
	void spawnLobby3D(class ASpacelPlayerState const* _owningPlayerState);
	
	/* setup method */
	void setupColor(class ASpacelPlayerState const* _owningPlayerState);
	void setupEnnemyTeam();
	void setupInGame();
	void setupChooseSkill();

	/* tick method */
	void tickTimer(float _deltaSeconde);
	void tickScore();

	/* skill method */
	void addSkill(class SkillCountDown* _skill);
	void removeSkill(ESkill _type);

	/* mission */
	void startMission(FMission * _mission);
	void updateMissionArrowOrientation();

protected:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UTeamColorDataAsset* TeamColorDataAsset { nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class USkillDataAsset* SkillDataAsset { nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UFlyingGameModeDataAsset* FlyingModeDataAsset { nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UKeyDataAsset* KeyDataAsset { nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UMissionDataAsset* MissionDataAsset { nullptr };

	UPROPERTY(Category = "Hack", EditAnywhere, BlueprintReadWrite)
	class UEditorHackDataAsset* HackDataAsset{ nullptr };

	UPROPERTY(Category = "Setup", EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AActor> LobbyClass { nullptr };

private:
	/* timer part */
	float m_currentTimer { 0.0f };

	/* mission */
	class AActor* m_arrowTarget { nullptr };
};
