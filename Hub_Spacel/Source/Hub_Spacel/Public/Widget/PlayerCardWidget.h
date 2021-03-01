// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerCardWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UPlayerCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(class ASpacelPlayerState* _playerState);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY()
	class UTextBlock* PlayerNameTextBlock { nullptr };

	UPROPERTY()
	class UTextBlock* AttackTextBlock { nullptr };

	UPROPERTY()
	class UTextBlock* ProtectionTextBlock{ nullptr };

	UPROPERTY()
	class UTextBlock* SupportTextBlock{ nullptr };
};
