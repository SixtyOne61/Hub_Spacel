// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Enum/SpacelEnum.h"
#include "AllyWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UAllyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetWatcher(class ASpacelPlayerState* _state);

protected:
	void NativeConstruct() override;
	void NativeDestruct() override;

	void updatePercent(class UProgressBar* _progressBar, float _value);

	UFUNCTION()
	void UpdateWatcher();

protected:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UTeamColorDataAsset* TeamColorDataAsset{ nullptr };

private:
	UPROPERTY()
	class UTextBlock* NameTextBlock { nullptr };

	UPROPERTY()
	class UProgressBar* ProtectionProgressBar { nullptr };

	UPROPERTY()
	class UImage* DisconnectImage { nullptr };

	TWeakObjectPtr<class ASpacelPlayerState> m_watcher {};

	UPROPERTY()
	FTimerHandle WatcherHandle {};
};
