// Fill out your copyright notice in the Description page of Project Settings.
#include "Hub_SpacelGameInstance.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Hud/TextReaderComponent.h"

UHub_SpacelGameInstance::UHub_SpacelGameInstance()
{
    UTextReaderComponent* textReader = CreateDefaultSubobject<UTextReaderComponent>(TEXT("TextReaderComponent"));
    if (!ensure(textReader != nullptr)) return;

    ApiUrl = textReader->ReadFile("Urls/ApiUrl.txt");
    HttpModule = &FHttpModule::Get();
}

void UHub_SpacelGameInstance::Shutdown()
{
    Super::Shutdown();

    if (this->AccessToken.Len() > 0)
    {
        TSharedRef<IHttpRequest> invalidateTokensRequest = this->HttpModule->CreateRequest();
        invalidateTokensRequest->SetURL(this->ApiUrl + "/invalidatetokens");
        invalidateTokensRequest->SetVerb("GET");
        invalidateTokensRequest->SetHeader("Content-Type", "application/json");
        invalidateTokensRequest->SetHeader("Authorization", this->AccessToken);
        invalidateTokensRequest->ProcessRequest();
    }
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
            TSharedRef<IHttpRequest> retrieveNewTokensRequest { this->HttpModule->CreateRequest() };
            retrieveNewTokensRequest->OnProcessRequestComplete().BindUObject(this, &UHub_SpacelGameInstance::onRetrieveNewTokensResponseReceived);
            retrieveNewTokensRequest->SetURL(this->ApiUrl + "/retrievenewtokens");
            retrieveNewTokensRequest->SetVerb("POST");
            retrieveNewTokensRequest->SetHeader("Content-Type", "application/json");
            retrieveNewTokensRequest->SetHeader("Authorization", this->AccessToken);
            retrieveNewTokensRequest->SetContentAsString(requestBody);
            retrieveNewTokensRequest->ProcessRequest();
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
            if (!jsonObject->HasField("error"))
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