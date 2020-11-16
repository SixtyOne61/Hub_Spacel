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
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void HandleLoginUrlChange();

	UFUNCTION()
	void SetAveragePlayerLatency();

	UFUNCTION()
	void PollMatchmaking();

	void onExchangeCodeForTokensResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful);
	void onGetPlayerDataResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful);
	void onStartMatchmakingResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful);
	void onStopMatchmakingResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful);
	void onPollMatchmakingReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful);

	UFUNCTION()
	void OnMatchmakingButtonClicked();

	template<class T>
	T* initSafetyFromName(const FName& _name) const
	{
		T* obj = (T*)GetWidgetFromName(_name);
		if (!ensure(obj != nullptr)) return nullptr;
		return obj;
	}

	void setMatchkingTextBlock(TOptional<FText> && _button, TOptional<FText> && _event);

public:
	UPROPERTY()
	FTimerHandle SetAveragePlayerLatencyHandle {};

	UPROPERTY()
	FTimerHandle PollMatchmakingHandle {};

private:
	class FHttpModule* HttpModule { nullptr };

	UPROPERTY()
	FString LoginUrl {};

	UPROPERTY()
	FString ApiUrl {};

	UPROPERTY()
	FString CallbackUrl {};

	UPROPERTY()
	FString RegionCode {};

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
	float AveragePlayerLatency {};

	UPROPERTY()
	bool SearchingForGame {};
};
