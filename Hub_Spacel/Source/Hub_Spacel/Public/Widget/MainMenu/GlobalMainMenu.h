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

	struct SPlayerProfile
	{
		FString m_playerName {};
		FString m_win {};
		FString m_ratio {};
		FString m_nbGame {};
	} m_playerProfile;
	
public:
	UGlobalMainMenu(FObjectInitializer const& _objectInitializer);

protected:
	void NativeConstruct() override;
	void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent)
	void BP_MatchmakingState(FString const& _title);

	UFUNCTION(BlueprintImplementableEvent)
	void BP_Timer(bool _start);

private:
	// callback handle timer
	UFUNCTION()
	void SetAveragePlayerLatency();

	UFUNCTION()
	void HandleLoginUrlChange();

	UFUNCTION()
	void PollMatchmaking();

	//

	void onExchangeCodeForTokensResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful);
	void onGetPlayerDataResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful);
	void onStartMatchmakingResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful);
	void onStopMatchmakingResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful);
	void onPollMatchmakingReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful);

	UFUNCTION(BlueprintCallable, Category = "UIEvent")
	bool OnPlay();

	UFUNCTION(BlueprintCallable, Category = "UIEvent")
	void OnRefreshInput(class UInputSettings* _inputSettings);

	UFUNCTION(BlueprintCallable, Category = "UISetup")
	void SetPlayerProfile(FString & _playerName, FString & _win, FString& _ratio, FString& _nbGame);

	UFUNCTION(BlueprintCallable, Category = "UISetup")
	void RefreshPlayerName();

	UFUNCTION()
	void OnLoadGame(const FString& _slotName, const int32 _userIndex, USaveGame* _loadedGameData);

	// start / cancel match making
	void startSearch();
	void cancelSearch();

public:
	UPROPERTY()
	FTimerHandle SetAveragePlayerLatencyHandle {};

	UPROPERTY()
	FTimerHandle PollMatchmakingHandle {};

protected:
	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class USkillDataAsset* SkillDataAsset{ nullptr };

	UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
	class UHideSkillDataAsset* HideSkillDataAsset{ nullptr };

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

	bool SearchingForGame {};
};
