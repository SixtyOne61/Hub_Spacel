// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/SpacelEnum.h"
#include "DeathWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UDeathWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	void NativeConstruct() override;
	void NativeTick(const FGeometry& _myGeometry, float _deltaSeconde) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Setup")
	void BP_UpdateTimer(int _seconde);

	UFUNCTION(BlueprintImplementableEvent, Category = "Setup")
	void BP_EnableRespawn();

	UFUNCTION(BlueprintImplementableEvent, Category = "Setup")
	void BP_Setup(ESkill _low, ESkill _medium, ESkill _hight, FString const& _name, class UTexture2D* _logo, FColor const& _color);

	UFUNCTION(BlueprintImplementableEvent, Category = "Setup")
	void BP_KillByEnvironment();

	UFUNCTION(BlueprintImplementableEvent, Category = "Setup")
	void BP_AddMissionInProgress(class UTexture2D* _logo, FString const& _title, FString const& _desc);

	UFUNCTION(BlueprintCallable, Category = "Action")
	void Respawn();

	UFUNCTION(BlueprintImplementableEvent, Category = "Event")
	void BP_OnAddEffect(EEffect _effect);

	UFUNCTION()
	void WaitPlayerInfo();

private:
	/* update timer before enable respawn */
	void updateTimer(float _deltaSeconde);

	/* find mission un spacel game state and display information */
	void setupMissionInProgress(class ASpacelGameState* _spacelGameState);

protected:
	/* timer before enable respawn */
	float m_currentTimer { 0.0f };

protected:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UTeamColorDataAsset* TeamColorDataAsset { nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class USkillDataAsset* SkillDataAsset { nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UMissionDataAsset* MissionDataAsset { nullptr };
};
