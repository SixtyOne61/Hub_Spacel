// Fill out your copyright notice in the Description page of Project Settings.


#include "GlobalMainMenu.h"
#include "Hud/TextReaderComponent.h"
#include "WebBrowser.h"
#include "WebBrowserModule.h"
#include "IWebBrowserSingleton.h"
#include "IWebBrowserCookieManager.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Util/SimplyUI.h"
#include "Util/SimplyHttpRequest.h"
#include "Components/TextBlock.h"
#include "Hub_SpacelGameInstance.h"

UGlobalMainMenu::UGlobalMainMenu(FObjectInitializer const& _objectInitializer)
    : Super(_objectInitializer)
{
    UTextReaderComponent* textReader{ CreateDefaultSubobject<UTextReaderComponent>(TEXT("TextReaderComponent")) };
    if (!ensure(textReader != nullptr)) return;

    LoginUrl = textReader->ReadFile("Urls/LoginUrl.txt");
    ApiUrl = textReader->ReadFile("Urls/ApiUrl.txt");
    CallbackUrl = textReader->ReadFile("Urls/CallbackUrl.txt");
    RegionCode = textReader->ReadFile("Urls/RegionCode.txt");

    HttpModule = &FHttpModule::Get();

    AveragePlayerLatency = 60.0f;
}

void UGlobalMainMenu::NativeConstruct()
{
    Super::NativeConstruct();
    bIsFocusable = true;

    WebBrowser = SimplyUI::initSafetyFromName<UUserWidget, UWebBrowser>(this, TEXT("WebBrowser_Login"));

    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    world->GetTimerManager().SetTimer(SetAveragePlayerLatencyHandle, this, &UGlobalMainMenu::SetAveragePlayerLatency, 1.0f, true, 1.0f);

    UHub_SpacelGameInstance* spacelGameInstance{ Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
    if (!ensure(spacelGameInstance != nullptr)) return;
    FString accessToken = spacelGameInstance->AccessToken;
    if (accessToken.Len() > 0)
    {
        SimplyHttpRequest::processRequest(this->HttpModule, this,
            &UGlobalMainMenu::onGetPlayerDataResponseReceived,
            this->ApiUrl + "/getplayerdata", "GET",
            TArray<FString>{"Authorization", accessToken}, {});
    }
    else
    {
        IWebBrowserSingleton* webBrowserSingleton{ IWebBrowserModule::Get().GetSingleton() };
        if (!ensure(webBrowserSingleton != nullptr)) return;

        TOptional<FString> defaultContext{ };
        TSharedPtr<IWebBrowserCookieManager> cookieManager{ webBrowserSingleton->GetCookieManager(defaultContext) };

        if (cookieManager.IsValid())
        {
            cookieManager->DeleteCookies();
        }

        WebBrowser->LoadURL(LoginUrl);

        FScriptDelegate loginDelegate;
        loginDelegate.BindUFunction(this, "HandleLoginUrlChange");
        WebBrowser->OnUrlChanged.Add(loginDelegate);
    }

    //FAsyncLoadGameFromSlotDelegate LoadedDelegate;
    //// USomeUObjectClass::LoadGameDelegateFunction is a void function that takes the following parameters: const FString& SlotName, const int32 UserIndex, USaveGame* LoadedGameData
    //LoadedDelegate.BindUObject(this, &UMainMenuWidget::OnLoadGame);
    //UGameplayStatics::AsyncLoadGameFromSlot("Save", 0, LoadedDelegate);
}

void UGlobalMainMenu::NativeDestruct()
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    //world->GetTimerManager().ClearTimer(this->PollMatchmakingHandle);
    //world->GetTimerManager().ClearTimer(this->SetAveragePlayerLatencyHandle);
}

void UGlobalMainMenu::SetAveragePlayerLatency()
{
    UHub_SpacelGameInstance* spacelGameInstance{ Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
    if (!ensure(spacelGameInstance != nullptr)) return;

    float totalPlayerLatency{ 0.0f };
    for (float playerLatency : spacelGameInstance->PlayerLatencies)
    {
        totalPlayerLatency += playerLatency;
    }

    int32 num = spacelGameInstance->PlayerLatencies.Num();
    if (totalPlayerLatency > 0.0f && num != 0)
    {
        this->AveragePlayerLatency = totalPlayerLatency / num;
        FString pingString{ FString::FromInt(FMath::RoundToInt(this->AveragePlayerLatency)) + " ms" };
    }
}

void UGlobalMainMenu::HandleLoginUrlChange()
{
    if (!ensure(this->WebBrowser != nullptr)) return;
    FString browserUrl{ this->WebBrowser->GetUrl() };
    FString url{}, queryParameters{};

    if (browserUrl.Split("?", &url, &queryParameters))
    {
        if (url.Equals(this->CallbackUrl))
        {
            FString parameterName{}, parameterValue{};

            if (queryParameters.Split("=", &parameterName, &parameterValue))
            {
                if (parameterName.Equals("code"))
                {
                    parameterValue = parameterValue.Replace(*FString("#"), *FString(""));

                    TSharedPtr<FJsonObject> requestObj{ MakeShareable(new FJsonObject) };
                    requestObj->SetStringField(parameterName, parameterValue);

                    FString requestBody{};
                    TSharedRef<TJsonWriter<>> writer{ TJsonWriterFactory<>::Create(&requestBody) };

                    if (FJsonSerializer::Serialize(requestObj.ToSharedRef(), writer))
                    {
                        SimplyHttpRequest::processRequest(this->HttpModule, this,
                            &UGlobalMainMenu::onExchangeCodeForTokensResponseReceived,
                            this->ApiUrl + "/exchangecodefortokens", "POST",
                            TArray<FString>{"Content-Type", "application/json"}, requestBody);
                    }
                }
            }
        }
    }
}

void UGlobalMainMenu::onExchangeCodeForTokensResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful)
{
    if (!_bWasSuccessful)
    {
        return;
    }

    TSharedPtr<FJsonObject> jsonObject;
    TSharedRef<TJsonReader<>> reader{ TJsonReaderFactory<>::Create(_response->GetContentAsString()) };

    if (!FJsonSerializer::Deserialize(reader, jsonObject))
    {
        return;
    }

    if (!jsonObject->HasField("access_token")
        || !jsonObject->HasField("id_token")
        || !jsonObject->HasField("refresh_token"))
    {
        return;
    }

    UHub_SpacelGameInstance* spacelGameInstance{ Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
    if (!ensure(spacelGameInstance != nullptr)) return;

    FString accessToken{ jsonObject->GetStringField("access_token") };
    FString idToken{ jsonObject->GetStringField("id_token") };
    FString refreshToken{ jsonObject->GetStringField("refresh_token") };

    spacelGameInstance->SetCognitoTokens(accessToken, idToken, refreshToken);

    SimplyHttpRequest::processRequest(this->HttpModule, this,
        &UGlobalMainMenu::onGetPlayerDataResponseReceived,
        this->ApiUrl + "/getplayerdata", "GET",
        TArray<FString>{"Authorization", accessToken}, {});
}

void UGlobalMainMenu::onGetPlayerDataResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful)
{
    if (!_bWasSuccessful)
    {
        return;
    }

    TSharedPtr<FJsonObject> jsonObject;
    TSharedRef<TJsonReader<>> reader{ TJsonReaderFactory<>::Create(_response->GetContentAsString()) };

    if (!FJsonSerializer::Deserialize(reader, jsonObject))
    {
        return;
    }

    if (!jsonObject->HasField("playerData"))
    {
        return;
    }

    TSharedPtr<FJsonObject> playerData{ jsonObject->GetObjectField("playerData") };

    auto lb_set = [&playerData](UTextBlock* _text, FString&& _name)
    {
        if (!ensure(_text != nullptr)) return;
        if (playerData->HasField(_name))
        {
            FString txt = playerData->GetObjectField(_name)->GetStringField("N");
            _text->SetText(FText::FromString(_name + ": " + txt));
        }
    };

    //lb_set(this->WinsTextBlock, "Wins");
    //lb_set(this->LossesTextBlock, "Losses");
    //
    SimplyUI::setVisibility({ ESlateVisibility::Hidden },
        std::make_tuple(this->WebBrowser));
    //SimplyUI::setVisibility({ ESlateVisibility::Visible },
    //    std::make_tuple(this->MatchmakingButton, this->WinsTextBlock, this->LossesTextBlock, this->PingTextBlock, this->MatchmakingEventTextBlock, this->BorderName, this->ScrollBox, this->InputSelection));
}

void UGlobalMainMenu::onStartMatchmakingResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful)
{
    //this->MatchmakingButton->SetIsEnabled(true);

    if (!_bWasSuccessful)
    {
        return;
    }

    TSharedPtr<FJsonObject> jsonObject;
    TSharedRef<TJsonReader<>> reader{ TJsonReaderFactory<>::Create(_response->GetContentAsString()) };
    if (!FJsonSerializer::Deserialize(reader, jsonObject)
        || !jsonObject->HasField("ticketId"))
    {
        return;
    }

    FString matchmakingTicketId{ jsonObject->GetStringField("ticketId") };

    UHub_SpacelGameInstance* spacelGameInstance{ Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
    if (!ensure(spacelGameInstance != nullptr)) return;

    spacelGameInstance->MatchmakingTicketId = matchmakingTicketId;

    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    //world->GetTimerManager().SetTimer(this->PollMatchmakingHandle, this, &UMainMenuWidget::PollMatchmaking, 1.0f, true, 1.0f);
    //this->SearchingForGame = true;

    //setMatchkingTextBlock(FText::FromString("Cancel"), FText::FromString("Currently looking for a match"));
}

void UGlobalMainMenu::onStopMatchmakingResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful)
{
    UHub_SpacelGameInstance* spacelGameInstance{ Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
    if (!ensure(spacelGameInstance != nullptr)) return;

    spacelGameInstance->MatchmakingTicketId = {};

    //setMatchkingTextBlock(FText::FromString("Join Game"), FText::FromString(""));

    //this->MatchmakingButton->SetIsEnabled(true);
}

void UGlobalMainMenu::onPollMatchmakingReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful)
{
    /*if (!_bWasSuccessful || !this->SearchingForGame)
    {
        return;
    }

    TSharedPtr<FJsonObject> jsonObject;
    TSharedRef<TJsonReader<>> reader{ TJsonReaderFactory<>::Create(_response->GetContentAsString()) };
    if (!FJsonSerializer::Deserialize(reader, jsonObject)
        || !jsonObject->HasField("ticket"))
    {
        return;
    }

    TSharedPtr<FJsonObject> ticket{ jsonObject->GetObjectField("ticket") };
    FString ticketType{ ticket->GetObjectField("Type")->GetStringField("S") };

    if (ticketType.Len() > 0)
    {
        UWorld* world{ this->GetWorld() };
        if (!ensure(world != nullptr)) return;

        world->GetTimerManager().ClearTimer(this->PollMatchmakingHandle);
        this->SearchingForGame = false;

        UHub_SpacelGameInstance* spacelGameInstance{ Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
        if (!ensure(spacelGameInstance != nullptr)) return;
        spacelGameInstance->MatchmakingTicketId = {};

        if (ticketType.Equals("MatchmakingSucceeded"))
        {
            this->MatchmakingButton->SetIsEnabled(false);
            setMatchkingTextBlock({}, FText::FromString("Successfully found a match. Now connecting to the server..."));

            TSharedPtr<FJsonObject> gameSessionInfo{ ticket->GetObjectField("GameSessionInfo")->GetObjectField("M") };
            FString ipAddress{ gameSessionInfo->GetObjectField("IpAddress")->GetStringField("S") },
                port{ gameSessionInfo->GetObjectField("Port")->GetStringField("N") };

            TArray<TSharedPtr<FJsonValue>> players{ ticket->GetObjectField("Players")->GetArrayField("L") };
            TSharedPtr<FJsonObject> player{ players[0]->AsObject()->GetObjectField("M") };
            FString playerSessionId{ player->GetObjectField("PlayerSessionId")->GetStringField("S") },
                playerId{ player->GetObjectField("PlayerId")->GetStringField("S") };

            FString levelName{ ipAddress + ":" + port };
            FString const& options{ "?PlayerSessionId=" + playerSessionId + "?PlayerId=" + playerId };
            //UE_LOG(LogTemp, Warning, TEXT("options : %s"), *options);

            UGameplayStatics::OpenLevel(world, FName(*levelName), false, options);
        }
        else
        {
            setMatchkingTextBlock(FText::FromString("Join Game"), FText::FromString(ticketType + ". Please try again"));
        }
    }*/
}

void UGlobalMainMenu::OnPlay()
{
    UHub_SpacelGameInstance* spacelGameInstance { Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
    if (!ensure(spacelGameInstance != nullptr)) return;

    FString accessToken { spacelGameInstance->AccessToken };
    FString matchmakingTicketId { spacelGameInstance->MatchmakingTicketId };

    if (accessToken.Len() > 0)
    {
        TSharedRef<FJsonObject> latencyMapObj{ MakeShareable(new FJsonObject) };
        latencyMapObj->SetNumberField(this->RegionCode, this->AveragePlayerLatency);

        TSharedPtr<FJsonObject> requestObj{ MakeShareable(new FJsonObject) };
        requestObj->SetObjectField("latencyMap", latencyMapObj);

        FString requestBody;
        TSharedRef<TJsonWriter<>> writer = TJsonWriterFactory<>::Create(&requestBody);
        if (FJsonSerializer::Serialize(requestObj.ToSharedRef(), writer))
        {
            SimplyHttpRequest::processRequest(this->HttpModule, this,
                &UGlobalMainMenu::onStartMatchmakingResponseReceived,
                this->ApiUrl + "/startmatchmaking", "POST",
                TArray<FString>{"Content-Type", "application/json", "Authorization", accessToken},
                requestBody);
        }
    }
}