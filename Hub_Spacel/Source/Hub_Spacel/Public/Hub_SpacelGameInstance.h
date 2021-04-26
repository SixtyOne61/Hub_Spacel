// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Hub_SpacelGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTargetPlayer, int32, _playerId, bool, _lock);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTryLock);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUnlockUltimate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLooserTeamUnlockLvl2, FName const&, _loosingTeam);

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class HUB_SPACEL_API UHub_SpacelGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
    UHub_SpacelGameInstance();

    virtual void Shutdown() override;
    virtual void Init() override;

    UFUNCTION()
    void SetCognitoTokens(FString _accessToken, FString _idToken, FString _refreshToken);

private:
    UFUNCTION()
    void RetrieveNewTokens();
    
    UFUNCTION()
    void GetResponseTime();

    void onRetrieveNewTokensResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful);
    void onGetResponseTimeResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful);

public:
    UPROPERTY()
    FString AccessToken {};

    UPROPERTY()
    FString IdToken {};

    UPROPERTY()
    FString RefreshToken {};

    UPROPERTY()
    FString MatchmakingTicketId {};

    UPROPERTY()
    FTimerHandle RetrieveNewTokensHandle {};

    UPROPERTY()
    FTimerHandle GetResponseTimeHandle {};

    TDoubleLinkedList<float> PlayerLatencies {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnTargetPlayer OnTargetPlayerDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnTryLock OnTryLockDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnUnlockUltimate OnUnlockUltimateDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnLooserTeamUnlockLvl2 OnLooserTeamUnlockLvl2Delegate {};

    UPROPERTY()
    FString CustomPlayerName { "BoB" };

private:
    class FHttpModule* HttpModule { nullptr };

    UPROPERTY()
    FString ApiUrl {};

    UPROPERTY()
    FString RegionCode {};
};
