// Fill out your copyright notice in the Description page of Project Settings.
#include "MainMenuWidget.h"
#include "Hud/TextReaderComponent.h"
#include "WebBrowser.h"
#include "WebBrowserModule.h"
#include "IWebBrowserSingleton.h"
#include "IWebBrowserCookieManager.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Hub_SpacelGameInstance.h"
#include "Util/SimplyUI.h"
#include "Util/SimplyHttpRequest.h"

UMainMenuWidget::UMainMenuWidget(FObjectInitializer const& _objectInitializer)
    : Super(_objectInitializer)
{
    UTextReaderComponent* textReader { CreateDefaultSubobject<UTextReaderComponent>(TEXT("TextReaderComponent")) };
    if (!ensure(textReader != nullptr)) return;

    LoginUrl = textReader->ReadFile("Urls/LoginUrl.txt");
    ApiUrl = textReader->ReadFile("Urls/ApiUrl.txt");
    CallbackUrl = textReader->ReadFile("Urls/CallbackUrl.txt");

    HttpModule = &FHttpModule::Get();

    AveragePlayerLatency = 60.0f;
}

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();
    bIsFocusable = true;

    WebBrowser = initSafetyFromName<UWebBrowser>(TEXT("WebBrowser_Login"));
    MatchmakingButton = initSafetyFromName<UButton>(TEXT("Button_Matchmaking"));
    WinsTextBlock = initSafetyFromName<UTextBlock>(TEXT("TextBlock_Wins"));
    LossesTextBlock = initSafetyFromName<UTextBlock>(TEXT("TextBlock_Losses"));
    PingTextBlock = initSafetyFromName<UTextBlock>(TEXT("TextBlock_Ping"));
    MatchmakingEventTextBlock = initSafetyFromName<UTextBlock>(TEXT("TextBlock_MatchmakingEvent"));

    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    world->GetTimerManager().SetTimer(SetAveragePlayerLatencyHandle, this, &UMainMenuWidget::SetAveragePlayerLatency, 1.0f, true, 1.0f);

    IWebBrowserSingleton* webBrowserSingleton { IWebBrowserModule::Get().GetSingleton() };
    if (!ensure(webBrowserSingleton != nullptr)) return;

    TOptional<FString> defaultContext { };
    TSharedPtr<IWebBrowserCookieManager> cookieManager { webBrowserSingleton->GetCookieManager(defaultContext) };

    if (cookieManager.IsValid())
    {
        cookieManager->DeleteCookies();
    }

    WebBrowser->LoadURL(LoginUrl);

    FScriptDelegate loginDelegate;
    loginDelegate.BindUFunction(this, "HandleLoginUrlChange");
    WebBrowser->OnUrlChanged.Add(loginDelegate);
}

void UMainMenuWidget::HandleLoginUrlChange()
{
    if (!ensure(this->WebBrowser != nullptr)) return;
    FString browserUrl { this->WebBrowser->GetUrl() };
    FString url {}, queryParameters {};

    if (browserUrl.Split("?", &url, &queryParameters))
    {
        if (url.Equals(this->CallbackUrl))
        {
            FString parameterName {}, parameterValue {};

            if (queryParameters.Split("=", &parameterName, &parameterValue))
            {
                if (parameterName.Equals("code"))
                {
                    parameterValue = parameterValue.Replace(*FString("#"), *FString(""));
                    
                    TSharedPtr<FJsonObject> requestObj { MakeShareable(new FJsonObject) };
                    requestObj->SetStringField(parameterName, parameterValue);

                    FString requestBody {};
                    TSharedRef<TJsonWriter<>> writer { TJsonWriterFactory<>::Create(&requestBody) };

                    if (FJsonSerializer::Serialize(requestObj.ToSharedRef(), writer))
                    {
                        SimplyHttpRequest::processRequest(this->HttpModule, this,
                            &UMainMenuWidget::onExchangeCodeForTokensResponseReceived, 
                            this->ApiUrl + "/exchangecodefortokens", "POST", 
                            TArray<FString>{"Content-Type", "application/json"}, requestBody);
                    }
                }
            }
        }
    }
}

void UMainMenuWidget::onExchangeCodeForTokensResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful)
{
    if (!_bWasSuccessful)
    {
        return;
    }

    TSharedPtr<FJsonObject> jsonObject;
    TSharedRef<TJsonReader<>> reader { TJsonReaderFactory<>::Create(_response->GetContentAsString()) };

    if (!FJsonSerializer::Deserialize(reader, jsonObject))
    {
        return;
    }

    if (jsonObject->HasField("error"))
    {
        return;
    }

    UHub_SpacelGameInstance* spacelGameInstance { Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
    if (!ensure(spacelGameInstance != nullptr)) return;

    FString accessToken { jsonObject->GetStringField("access_token") };
    FString idToken { jsonObject->GetStringField("id_token") };
    FString refreshToken { jsonObject->GetStringField("refresh_token") };

    spacelGameInstance->SetCognitoTokens(accessToken, idToken, refreshToken);

    SimplyHttpRequest::processRequest(this->HttpModule, this,
        &UMainMenuWidget::onGetPlayerDataResponseReceived,
        this->ApiUrl + "/getplayerdata", "GET",
        TArray<FString>{"Authorization", accessToken}, {});
}

void UMainMenuWidget::onGetPlayerDataResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful)
{
    if (!_bWasSuccessful)
    {
        return;
    }

    TSharedPtr<FJsonObject> jsonObject;
    TSharedRef<TJsonReader<>> reader { TJsonReaderFactory<>::Create(_response->GetContentAsString()) };

    if (!FJsonSerializer::Deserialize(reader, jsonObject))
    {
        return;
    }

    if (jsonObject->HasField("error"))
    {
        return;
    }

    if (!jsonObject->HasField("playerData"))
    {
        return;
    }

    TSharedPtr<FJsonObject> playerData { jsonObject->GetObjectField("playerData") };

    auto lb_set = [&playerData](UTextBlock* _text, FString&& _name)
    {
        if (!ensure(_text != nullptr)) return;
        if (playerData->HasField(_name))
        {
            FString txt = playerData->GetObjectField(_name)->GetStringField("N");
            _text->SetText(FText::FromString(_name + ": " + txt));
        }
    };

    lb_set(this->WinsTextBlock, "Wins");
    lb_set(this->LossesTextBlock, "Losses");

    SimpleUI::setVisibility({ESlateVisibility::Hidden}, 
        std::make_tuple(this->WebBrowser));
    SimpleUI::setVisibility({ ESlateVisibility::Visible },
        std::make_tuple(this->MatchmakingButton, this->WinsTextBlock, this->LossesTextBlock, this->PingTextBlock, this->MatchmakingEventTextBlock));
}

void UMainMenuWidget::SetAveragePlayerLatency()
{
    UHub_SpacelGameInstance* spacelGameInstance { Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
    if (!ensure(spacelGameInstance != nullptr)) return;

    float totalPlayerLatency { 0.0f };
    for (float playerLatency : spacelGameInstance->PlayerLatencies)
    {
        totalPlayerLatency += playerLatency;
    }

    int32 num = spacelGameInstance->PlayerLatencies.Num();
    if (totalPlayerLatency > 0.0f && num != 0)
    {
        this->AveragePlayerLatency = totalPlayerLatency / num;
        FString pingString { "Ping: " + FString::FromInt(FMath::RoundToInt(this->AveragePlayerLatency)) + "ms" };
        if (!ensure(this->PingTextBlock != nullptr)) return;
        this->PingTextBlock->SetText(FText::FromString(pingString));
    }
}