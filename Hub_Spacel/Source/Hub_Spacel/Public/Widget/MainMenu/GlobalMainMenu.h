// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Http.h"
#include "GlobalMainMenu.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UGlobalMainMenu : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UGlobalMainMenu(FObjectInitializer const& _objectInitializer);

protected:
	void NativeConstruct() override;
	void NativeDestruct() override;

private:
	UFUNCTION()
	void SetAveragePlayerLatency();

	UFUNCTION()
	void HandleLoginUrlChange();

	void onExchangeCodeForTokensResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful);
	void onGetPlayerDataResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful);
	void onStartMatchmakingResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful);
	void onStopMatchmakingResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful);
	void onPollMatchmakingReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = "UIEvent")
	void OnPlay();

public:
	UPROPERTY()
	FTimerHandle SetAveragePlayerLatencyHandle {};

	UPROPERTY()
	FTimerHandle PollMatchmakingHandle {};

private:
	// widget component

private:
	class FHttpModule* HttpModule{ nullptr };

	UPROPERTY()
	FString LoginUrl {};

	UPROPERTY()
	FString ApiUrl {};

	UPROPERTY()
	FString CallbackUrl {};

	UPROPERTY()
	FString RegionCode {};

	UPROPERTY()
	class UWebBrowser* WebBrowser{ nullptr };

	UPROPERTY()
	float AveragePlayerLatency{};
};
