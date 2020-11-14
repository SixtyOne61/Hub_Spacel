// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Http.h"
#include "MainMenuWidget.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMainMenuWidget(FObjectInitializer const& _objectInitializer);
	
protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void HandleLoginUrlChange();

	UFUNCTION()
	void SetAveragePlayerLatency();

	void onExchangeCodeForTokensResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful);
	void onGetPlayerDataResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful);

	template<class T>
	T* initSafetyFromName(const FName& _name) const
	{
		T* obj = (T*)GetWidgetFromName(_name);
		if (!ensure(obj != nullptr)) return nullptr;
		return obj;
	}

public:
	UPROPERTY()
	FTimerHandle SetAveragePlayerLatencyHandle;

private:
	class FHttpModule* HttpModule { nullptr };

	UPROPERTY()
	FString LoginUrl {};

	UPROPERTY()
	FString ApiUrl {};

	UPROPERTY()
	FString CallbackUrl {};

	UPROPERTY()
	class UWebBrowser* WebBrowser { nullptr };

	UPROPERTY()
	class UButton* MatchmakingButton { nullptr };

	UPROPERTY()
	class UTextBlock* WinsTextBlock { nullptr };

	UPROPERTY()
	class UTextBlock* LossesTextBlock { nullptr };

	UPROPERTY()
	class UTextBlock* PingTextBlock { nullptr };

	UPROPERTY()
	class UTextBlock* MatchmakingEventTextBlock { nullptr };

	UPROPERTY()
	float AveragePlayerLatency;
};
