// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "GameState/SpacelGameState.h"
#include "Hub_SpacelGameInstance.generated.h"

USTRUCT(BlueprintType)
struct HUB_SPACEL_API FMetricData
{
	GENERATED_BODY()

    FMetricData() = default;

    FMetricData(FPlayerData const& _data, int _kill, int _death, int _assist, int _score)
        : PlayerInfo(_data)
        , Kill(_kill)
        , Death(_death)
        , Assist(_assist)
        , Score(_score)
    {}

	UPROPERTY(BlueprintReadOnly)
    FPlayerData PlayerInfo { };

    UPROPERTY(BlueprintReadOnly)
    int Kill { 0 };

    UPROPERTY(BlueprintReadOnly)
    int Death { 0 };

    UPROPERTY(BlueprintReadOnly)
    int Assist { 0 };

    UPROPERTY(BlueprintReadOnly)
    int Score { 0 };
};

/**
 * Each client or server has their own game instance
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

    UPROPERTY(BlueprintReadWrite)
    FString CustomPlayerName { "BoB" };

    UPROPERTY()
    TArray<FMetricData> MetricPlayersData {};

    UPROPERTY()
    TArray<FScore> ScoresData {};

private:
    class FHttpModule* HttpModule { nullptr };

    UPROPERTY()
    FString ApiUrl {};

    UPROPERTY()
    FString RegionCode {};
};
