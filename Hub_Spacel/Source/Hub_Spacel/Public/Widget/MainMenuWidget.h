// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Runtime/Online/HTTP/Public/Http.h"
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

	void onExchangeCodeForTokensResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful);

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
};
