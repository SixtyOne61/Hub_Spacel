// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SpacelHUD.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API ASpacelHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	ASpacelHUD();

protected:
	void BeginPlay() override;
	void Tick(float _deltaSeconde) override;

	UFUNCTION(BlueprintImplementableEvent)
	void BP_UpdateAlpha(float _alpha);

private:
	UFUNCTION()
	void OnChangeState(EGameState _state);

	UFUNCTION()
	void OnLocalPlayerAddEffect(EEffect _effect);

	UFUNCTION()
	void OnLocalPlayerRemoveEffect(EEffect _effect);

	UFUNCTION()
	void OnScored(EScoreType _type, int32 _value);

private:
	/* setup cursor */
	void switchToGameAndUI();
	void switchToUIOnly();

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> InGameWidgetClass { nullptr };

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> EndMenuWidgetClass { nullptr };

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> DeathWidgetClass{ nullptr };

private:
	/* alpha for hit marker feedback */
	float m_alpha { 0.0f };

	/* timer for lerp anim */
	float m_timer { 0.0f };
};
