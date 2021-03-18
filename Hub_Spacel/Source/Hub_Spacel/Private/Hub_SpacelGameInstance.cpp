// Fill out your copyright notice in the Description page of Project Settings.
#include "Hub_SpacelGameInstance.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Hud/TextReaderComponent.h"
#include "Util/SimplyHttpRequest.h"

UHub_SpacelGameInstance::UHub_SpacelGameInstance()
{
    UTextReaderComponent* textReader = CreateDefaultSubobject<UTextReaderComponent>(TEXT("TextReaderComponent"));
    if (!ensure(textReader != nullptr)) return;

    ApiUrl = textReader->ReadFile("Urls/ApiUrl.txt");
    RegionCode = textReader->ReadFile("Urls/RegionCode.txt");
    HttpModule = &FHttpModule::Get();
}

void UHub_SpacelGameInstance::Shutdown()
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    world->GetTimerManager().ClearTimer(this->RetrieveNewTokensHandle);
    world->GetTimerManager().ClearTimer(this->GetResponseTimeHandle);

    if (this->AccessToken.Len() > 0)
    {
        if (this->MatchmakingTicketId.Len() > 0)
        {
            TSharedPtr<FJsonObject> requestObj { MakeShareable(new FJsonObject) };
            requestObj->SetStringField("ticketId", this->MatchmakingTicketId);

            FString requestBody;
            TSharedRef<TJsonWriter<>> writer = TJsonWriterFactory<>::Create(&requestBody);
            if (FJsonSerializer::Serialize(requestObj.ToSharedRef(), writer))
            {
                SimplyHttpRequest::processRequest(this->HttpModule, this->ApiUrl + "/stopmatchmaking",
                    "POST", TArray<FString>{"Content-Type", "application/json", "Authorization", this->AccessToken}, requestBody);
            }
        }

        SimplyHttpRequest::processRequest(this->HttpModule,
            this->ApiUrl + "/invalidatetokens", "GET",
            TArray<FString>{"Authorization", this->AccessToken}, {});
    }

    Super::Shutdown();
}

void UHub_SpacelGameInstance::Init()
{
    Super::Init();

    UWorld* world { this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    world->GetTimerManager().SetTimer(this->GetResponseTimeHandle, this, &UHub_SpacelGameInstance::GetResponseTime, 1.0f, true, 1.0f);
}

void UHub_SpacelGameInstance::SetCognitoTokens(FString _accessToken, FString _idToken, FString _refreshToken)
{
    this->AccessToken = _accessToken ;
    this->IdToken = _idToken;
    this->RefreshToken = _refreshToken;

    //UE_LOG(LogTemp, Warning, TEXT("access token: %s"), *this->AccessToken);
    //UE_LOG(LogTemp, Warning, TEXT("refresh token: %s"), *this->RefreshToken);

    UWorld* world { this->GetWorld() };
    if (!ensure(world != nullptr)) return;
    world->GetTimerManager().SetTimer(this->RetrieveNewTokensHandle, this, &UHub_SpacelGameInstance::RetrieveNewTokens, 1.0f, false, 3300.0f);
}

void UHub_SpacelGameInstance::RetrieveNewTokens()
{
    if (this->AccessToken.Len() > 0 && this->RefreshToken.Len() > 0)
    {
        TSharedPtr<FJsonObject> requestObj { MakeShareable(new FJsonObject) };
        requestObj->SetStringField("refreshToken", RefreshToken);

        FString requestBody {};
        TSharedRef<TJsonWriter<>> writer { TJsonWriterFactory<>::Create(&requestBody) };

        if (FJsonSerializer::Serialize(requestObj.ToSharedRef(), writer))
        {
            SimplyHttpRequest::processRequest(this->HttpModule, this,
                &UHub_SpacelGameInstance::onRetrieveNewTokensResponseReceived,
                this->ApiUrl + "/retrievenewtokens", "POST",
                TArray<FString>{"Content-Type", "application/json", "Authorization", this->AccessToken}, requestBody);
        }
        else
        {
            UWorld* world{ this->GetWorld() };
            if (!ensure(world != nullptr)) return;
            world->GetTimerManager().SetTimer(this->RetrieveNewTokensHandle, this, &UHub_SpacelGameInstance::RetrieveNewTokens, 1.0f, false, 30.0f);
        }
    }
}

void UHub_SpacelGameInstance::onRetrieveNewTokensResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful)
{
    if (_bWasSuccessful)
    {
        TSharedPtr<FJsonObject> jsonObject;
        TSharedRef<TJsonReader<>> reader { TJsonReaderFactory<>::Create(_response->GetContentAsString()) };
        if (FJsonSerializer::Deserialize(reader, jsonObject))
        {
            if (jsonObject->HasField("accessToken") && jsonObject->HasField("idToken"))
            {
                SetCognitoTokens(jsonObject->GetStringField("accessToken"), jsonObject->GetStringField("idToken"), this->RefreshToken);
            }
        }
        else
        {
            UWorld* world{ this->GetWorld() };
            if (!ensure(world != nullptr)) return;
            world->GetTimerManager().SetTimer(this->RetrieveNewTokensHandle, this, &UHub_SpacelGameInstance::RetrieveNewTokens, 1.0f, false, 30.0f);
        }
    }
    else
    {
        UWorld* world{ this->GetWorld() };
        if (!ensure(world != nullptr)) return;
        world->GetTimerManager().SetTimer(this->RetrieveNewTokensHandle, this, &UHub_SpacelGameInstance::RetrieveNewTokens, 1.0f, false, 30.0f);
    }
}

void UHub_SpacelGameInstance::GetResponseTime()
{
    SimplyHttpRequest::processRequest(this->HttpModule, this,
        &UHub_SpacelGameInstance::onGetResponseTimeResponseReceived,
        "https://gamelift." + this->RegionCode + ".amazonaws.com",
        "GET", {}, {});
}

void UHub_SpacelGameInstance::onGetResponseTimeResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful)
{
    if (this->PlayerLatencies.Num() >= 4)
    {
        this->PlayerLatencies.RemoveNode(PlayerLatencies.GetHead());
    }

    float responseTime = _request->GetElapsedTime() * 1000;
    //UE_LOG(LogTemp, Warning, TEXT("response time in milliseconds: %s"), *FString::SanitizeFloat(responseTime));

    this->PlayerLatencies.AddTail(responseTime);
}
