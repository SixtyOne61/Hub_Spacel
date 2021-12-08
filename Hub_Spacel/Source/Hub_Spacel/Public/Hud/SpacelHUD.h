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
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void OnChangeState(EGameState _state);

	UFUNCTION()
	void OnLocalPlayerAddEffect(EEffect _effect);

	UFUNCTION()
	void OnLocalPlayerRemoveEffect(EEffect _effect);

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
};
