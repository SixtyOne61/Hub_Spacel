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
	
protected:
	void NativeConstruct() override;
	void NativeDestruct() override;

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
	void BP_SetupSkill(ESkillType _type, UTexture2D * _icon);

private:
	UFUNCTION()
	void OnChangeState(EGameState _state);

	UFUNCTION()
	void WaitPlayerState();

private:
	void spawnLobby3D(class ASpacelPlayerState const* _owningPlayerState);
	void setupColor(class ASpacelPlayerState const* _owningPlayerState);

protected:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UTeamColorDataAsset* TeamColorDataAsset { nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class USkillDataAsset* SkillDataAsset { nullptr };

	UPROPERTY(Category = "Setup", EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class AActor> LobbyClass { nullptr };

private:
	ESkillType m_currentSkillType {};
};
