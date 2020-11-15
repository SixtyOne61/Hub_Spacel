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
    Super::Shutdown();

    if (this->AccessToken.Len() > 0)
    {
        SimplyHttpRequest::processRequest(this->HttpModule,
            this->ApiUrl + "/invalidatetokens", "GET",
            TArray<FString>{"Content-Type", "application/json", "Authorization", this->AccessToken}, {});

        //TSharedRef<IHttpRequest> invalidateTokensRequest = this->HttpModule->CreateRequest();
        //invalidateTokensRequest->SetURL(this->ApiUrl + "/invalidatetokens");
        //invalidateTokensRequest->SetVerb("GET");
        //invalidateTokensRequest->SetHeader("Content-Type", "application/json");
        //invalidateTokensRequest->SetHeader("Authorization", this->AccessToken);
        //invalidateTokensRequest->ProcessRequest();
    }
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

void UHub_SpacelGameInstance::GetResponseTime()
{
    TSharedRef<IHttpRequest> getResponseTimeRequest = this->HttpModule->CreateRequest();
    getResponseTimeRequest->OnProcessRequestComplete().BindUObject(this, &UHub_SpacelGameInstance::onGetResponseTimeResponseReceived);
    getResponseTimeRequest->SetURL("https://gamelift." + this->RegionCode + ".amazonaws.com");
    getResponseTimeRequest->SetVerb("GET");
    getResponseTimeRequest->ProcessRequest();
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