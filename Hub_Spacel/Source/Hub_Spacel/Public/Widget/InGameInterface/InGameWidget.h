// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/SpacelEnum.h"
#include "InGameWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UInGameWidget : public UUserWidget
{
	GENERATED_BODY()

	enum class EInternState : uint8
	{
		ChooseLow,
		ChooseMedium,
		ChooseHight,
		Go,
		InGame
	};
	
protected:
	void NativeConstruct() override;
	void NativeDestruct() override;
	void NativeTick(const FGeometry& _myGeometry, float _deltaSeconde) override;

	UFUNCTION()
	void OnChangeCarrousel(ESkill _skillId, ESkillType _type);

	UFUNCTION()
	void OnHoverCarrousel(ESkill _skillId, ESkillType _type);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetupOutline(FSlateColor _color);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetupSkillCarrousel(ESkillType _type);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetupSkill(ESkillType _type, UTexture2D * _icon, FSlateColor const& _backgroundColor);

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

private:
	UFUNCTION()
	void OnChangeState(EGameState _state);

	UFUNCTION()
	void WaitPlayerState();

private:
	/* spawn method */
	void spawnLobby3D(class ASpacelPlayerState const* _owningPlayerState);
	
	/* setup method */
	void setupColor(class ASpacelPlayerState const* _owningPlayerState);
	void setupEnnemyTeam();
	void setupInGame();

	/* tick method */
	void tickTimer(float _deltaSeconde);
	void tickScore();

	/* skill method */
	void addSkill(class SkillCountDown* _skill);
	void removeSkill(ESkill _type);

protected:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UTeamColorDataAsset* TeamColorDataAsset { nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class USkillDataAsset* SkillDataAsset { nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UFlyingGameModeDataAsset* FlyingModeDataAsset { nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UKeyDataAsset* KeyDataAsset { nullptr };

	UPROPERTY(Category = "Hack", EditAnywhere, BlueprintReadWrite)
	class UEditorHackDataAsset* HackDataAsset{ nullptr };

	UPROPERTY(Category = "Setup", EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AActor> LobbyClass { nullptr };

protected:
	UPROPERTY()
	class USkillCarrouselWidget* CarrouselWidget { nullptr };

private:
	// lobby part
	ESkillType m_currentSkillType {};

	// timer part
	EInternState m_internState { EInternState::ChooseLow };
	float m_currentTimer { 0.0f };
};
