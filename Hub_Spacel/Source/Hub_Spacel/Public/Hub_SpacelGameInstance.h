// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Hub_SpacelGameInstance.generated.h"

/**
 * 
 */
UCLASS()
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

private:
    class FHttpModule* HttpModule { nullptr };

    UPROPERTY()
    FString ApiUrl {};

    UPROPERTY()
    FString RegionCode {};
};
