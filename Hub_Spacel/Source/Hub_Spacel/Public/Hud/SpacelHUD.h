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
	void StartGame(EGameState _state);

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> GameWidgetClass { nullptr };

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> LobbyWidgetClass { nullptr };

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> EndMenuWidgetClass{ nullptr };
};
