// Fill out your copyright notice in the Description page of Project Settings.
#include "MainMenuWidget.h"
#include "Hud/TextReaderComponent.h"
#include "WebBrowser.h"
#include "WebBrowserModule.h"
#include "IWebBrowserSingleton.h"
#include "IWebBrowserCookieManager.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Hub_SpacelGameInstance.h"

UMainMenuWidget::UMainMenuWidget(FObjectInitializer const& _objectInitializer)
    : Super(_objectInitializer)
{
    UTextReaderComponent* textReader = CreateDefaultSubobject<UTextReaderComponent>(TEXT("TextReaderComponent"));
    if (!ensure(textReader != nullptr)) return;

    LoginUrl = textReader->ReadFile("Urls/LoginUrl.txt");
    ApiUrl = textReader->ReadFile("Urls/ApiUrl.txt");
    CallbackUrl = textReader->ReadFile("Urls/CallbackUrl.txt");

    HttpModule = &FHttpModule::Get();
}

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();
    bIsFocusable = true;

    WebBrowser = (UWebBrowser*)GetWidgetFromName(TEXT("WebBrowser_Login"));
    if (!ensure(WebBrowser != nullptr)) return;

    IWebBrowserSingleton* webBrowserSingleton = IWebBrowserModule::Get().GetSingleton();
    if (!ensure(webBrowserSingleton != nullptr)) return;

    TOptional<FString> defaultContext;
    TSharedPtr<IWebBrowserCookieManager> cookieManager = webBrowserSingleton->GetCookieManager(defaultContext);

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
                    
                    TSharedPtr<FJsonObject> requestObj = MakeShareable(new FJsonObject);
                    requestObj->SetStringField(parameterName, parameterValue);

                    FString requestBody {};
                    TSharedRef<TJsonWriter<>> writer = TJsonWriterFactory<>::Create(&requestBody);

                    if (FJsonSerializer::Serialize(requestObj.ToSharedRef(), writer))
                    {
                        TSharedRef<IHttpRequest> exchangeCodeForTokensRequest { this->HttpModule->CreateRequest() };
                        exchangeCodeForTokensRequest->OnProcessRequestComplete().BindUObject(this, &UMainMenuWidget::onExchangeCodeForTokensResponseReceived);
                        exchangeCodeForTokensRequest->SetURL(this->ApiUrl + "/exchangecodefortokens");
                        exchangeCodeForTokensRequest->SetVerb("POST");
                        exchangeCodeForTokensRequest->SetHeader("Content-Type", "application/json");
                        exchangeCodeForTokensRequest->SetContentAsString(requestBody);
                        exchangeCodeForTokensRequest->ProcessRequest();
                    }
                }
            }
        }
    }
}

void UMainMenuWidget::onExchangeCodeForTokensResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful)
{
    if (_bWasSuccessful)
    {
        TSharedPtr<FJsonObject> jsonObject;
        TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(_response->GetContentAsString());
        if (FJsonSerializer::Deserialize(reader, jsonObject))
        {
            if (!jsonObject->HasField("error"))
            {
                UHub_SpacelGameInstance* spacelGameInstance = Cast<UHub_SpacelGameInstance>(this->GetGameInstance());
                if (!ensure(spacelGameInstance != nullptr)) return;

                spacelGameInstance->SetCognitoTokens(jsonObject->GetStringField("access_token"),
                                                    jsonObject->GetStringField("id_token"),
                                                    jsonObject->GetStringField("refresh_token"));
            }
        }
    }
}