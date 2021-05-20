// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/SpacelEnum.h"
#include "LobbyUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API ULobbyUserWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	void NativeConstruct() override;
	void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent)
	void BP_SetupOutline(FSlateColor _color);

private:
	UFUNCTION()
	void StartLobby(EGameState _state);

	UFUNCTION()
	void SetupOwningTeam();

	UFUNCTION()
	void SetTime();

	void setupSkill(TArray<ESkill> const& _skills);
	void saveSkillChoosen(uint8 _id, ESkillType _type);
	void setTimer(int _timer);

protected:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class USkillDataAsset* SkillDataAsset { nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UTeamColorDataAsset* Colors { nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UFlyingGameModeDataAsset* GameModeDataAsset { nullptr };

	UPROPERTY(Category = "Setup", EditAnywhere, BlueprintReadWrite)
	TArray<ESkill> LowSkill {};

	UPROPERTY(Category = "Setup", EditAnywhere, BlueprintReadWrite)
	TArray<ESkill> MediumSkill{}; 

	UPROPERTY(Category = "Setup", EditAnywhere, BlueprintReadWrite)
	TArray<ESkill> HightSkill{};

	UPROPERTY()
	class USpinCarrouselWidget* Carrousel { nullptr };

	UPROPERTY()
	class UTextBlock* TimeTextBlock{ nullptr };

	UPROPERTY()
	FTimerHandle TimeHandle {};

	int Time { 0 };
};
