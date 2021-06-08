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
#include "Components/Border.h"
#include "Components/EditableText.h"
#include "Hub_SpacelGameInstance.h"
#include "Util/SimplyUI.h"
#include "Util/SimplyHttpRequest.h"
#include "Kismet/GameplayStatics.h"
#include "Player/Save/SpacelSaveGame.h"

UMainMenuWidget::UMainMenuWidget(FObjectInitializer const& _objectInitializer)
    : Super(_objectInitializer)
{
    UTextReaderComponent* textReader { CreateDefaultSubobject<UTextReaderComponent>(TEXT("TextReaderComponent")) };
    if (!ensure(textReader != nullptr)) return;

    LoginUrl = textReader->ReadFile("Urls/LoginUrl.txt");
    ApiUrl = textReader->ReadFile("Urls/ApiUrl.txt");
    CallbackUrl = textReader->ReadFile("Urls/CallbackUrl.txt");
    RegionCode = textReader->ReadFile("Urls/RegionCode.txt");

    HttpModule = &FHttpModule::Get();

    AveragePlayerLatency = 60.0f;
    SearchingForGame = false;
}

void UMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();
    bIsFocusable = true;

    WebBrowser = SimplyUI::initSafetyFromName<UUserWidget, UWebBrowser>(this, TEXT("WebBrowser_Login"));
    MatchmakingButton = SimplyUI::initSafetyFromName<UUserWidget, UButton>(this, TEXT("Button_Matchmaking"));
    FScriptDelegate matchmakingDelegate {};
    matchmakingDelegate.BindUFunction(this, "OnMatchmakingButtonClicked");
    MatchmakingButton->OnClicked.Add(matchmakingDelegate);

    WinsTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_Wins"));
    LossesTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_Losses"));
    PingTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_Ping"));
    MatchmakingEventTextBlock = SimplyUI::initSafetyFromName<UUserWidget, UTextBlock>(this, TEXT("TextBlock_MatchmakingEvent"));
    BorderName = SimplyUI::initSafetyFromName<UUserWidget, UBorder>(this, TEXT("Border_Name"));
    PlayerName = SimplyUI::initSafetyFromName<UUserWidget, UEditableText>(this, TEXT("EditableText_Name"));
    if (PlayerName)
    {
        PlayerName->OnTextChanged.AddDynamic(this, &UMainMenuWidget::OnPlayerNameChange);
    }

    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    world->GetTimerManager().SetTimer(SetAveragePlayerLatencyHandle, this, &UMainMenuWidget::SetAveragePlayerLatency, 1.0f, true, 1.0f);

    UHub_SpacelGameInstance* spacelGameInstance{ Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
    if (!ensure(spacelGameInstance != nullptr)) return;
    FString accessToken = spacelGameInstance->AccessToken;
    if (accessToken.Len() > 0)
    {
        SimplyHttpRequest::processRequest(this->HttpModule, this,
            &UMainMenuWidget::onGetPlayerDataResponseReceived,
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

    FAsyncLoadGameFromSlotDelegate LoadedDelegate;
    // USomeUObjectClass::LoadGameDelegateFunction is a void function that takes the following parameters: const FString& SlotName, const int32 UserIndex, USaveGame* LoadedGameData
    LoadedDelegate.BindUObject(this, &UMainMenuWidget::OnLoadGame);
    UGameplayStatics::AsyncLoadGameFromSlot("Save", 0, LoadedDelegate);
}

void UMainMenuWidget::NativeDestruct()
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    world->GetTimerManager().ClearTimer(this->PollMatchmakingHandle);
    world->GetTimerManager().ClearTimer(this->SetAveragePlayerLatencyHandle);
}

void UMainMenuWidget::OnLoadGame(const FString& _slotName, const int32 _userIndex, USaveGame* _loadedGameData)
{
    if (USpacelSaveGame* spacelSaveGame = Cast<USpacelSaveGame>(_loadedGameData))
    {
        if (UHub_SpacelGameInstance* spacelGameInstance = Cast<UHub_SpacelGameInstance>(this->GetGameInstance()))
        {
            // Set data on the savegame object.
            spacelGameInstance->CustomPlayerName = spacelSaveGame->PlayerName;
            if (this->PlayerName != nullptr)
            {
                this->PlayerName->SetText(FText::FromString(spacelGameInstance->CustomPlayerName));
            }
        }
    }
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

    if (!jsonObject->HasField("access_token")
        || !jsonObject->HasField("id_token")
        || !jsonObject->HasField("refresh_token"))
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

    SimplyUI::setVisibility({ESlateVisibility::Hidden}, 
        std::make_tuple(this->WebBrowser));
    SimplyUI::setVisibility({ ESlateVisibility::Visible },
        std::make_tuple(this->MatchmakingButton, this->WinsTextBlock, this->LossesTextBlock, this->PingTextBlock, this->MatchmakingEventTextBlock, this->BorderName));
}

void UMainMenuWidget::OnMatchmakingButtonClicked()
{
    // prevent double click
    this->MatchmakingButton->SetIsEnabled(false);

    UHub_SpacelGameInstance* spacelGameInstance { Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
    if (!ensure(spacelGameInstance != nullptr)) return;

    FString accessToken { spacelGameInstance->AccessToken };
    FString matchmakingTicketId { spacelGameInstance->MatchmakingTicketId };

    if (this->SearchingForGame)
    {
        UWorld* world{ this->GetWorld() };
        if (!ensure(world != nullptr)) return;

        world->GetTimerManager().ClearTimer(this->PollMatchmakingHandle);

        FText buttonText{}, eventText{};
        this->SearchingForGame = false;

        if (accessToken.Len() > 0 && matchmakingTicketId.Len() > 0)
        {
            TSharedPtr<FJsonObject> requestObj { MakeShareable(new FJsonObject) };
            requestObj->SetStringField("ticketId", matchmakingTicketId);

            FString requestBody;
            TSharedRef<TJsonWriter<>> writer = TJsonWriterFactory<>::Create(&requestBody);
            if (FJsonSerializer::Serialize(requestObj.ToSharedRef(), writer))
            {
                SimplyHttpRequest::processRequest(this->HttpModule, this,
                &UMainMenuWidget::onStopMatchmakingResponseReceived,
                this->ApiUrl + "/stopmatchmaking", "POST",
                    TArray<FString>{"Content-Type", "application/json", "Authorization", accessToken},
                    requestBody);
            }
            else
            {
                buttonText = FText::FromString("Join Game");
                eventText = FText::FromString("");

                this->MatchmakingButton->SetIsEnabled(true);
            }
        }
        else
        {
            buttonText = FText::FromString("Join Game");
            eventText = FText::FromString("");

            this->MatchmakingButton->SetIsEnabled(true);
        }

        setMatchkingTextBlock(buttonText, eventText);
    }
    else
    {
        if (accessToken.Len() > 0)
        {
            TSharedRef<FJsonObject> latencyMapObj { MakeShareable(new FJsonObject) };
            latencyMapObj->SetNumberField(this->RegionCode, this->AveragePlayerLatency);

            TSharedPtr<FJsonObject> requestObj { MakeShareable(new FJsonObject) };
            requestObj->SetObjectField("latencyMap", latencyMapObj);

            FString requestBody;
            TSharedRef<TJsonWriter<>> writer = TJsonWriterFactory<>::Create(&requestBody);
            if (FJsonSerializer::Serialize(requestObj.ToSharedRef(), writer))
            {
                SimplyHttpRequest::processRequest(this->HttpModule, this,
                    &UMainMenuWidget::onStartMatchmakingResponseReceived,
                    this->ApiUrl + "/startmatchmaking", "POST",
                    TArray<FString>{"Content-Type", "application/json", "Authorization", accessToken},
                    requestBody);
            }
            else
            {
                this->MatchmakingButton->SetIsEnabled(true);
            }
        }
        else
        {
            this->MatchmakingButton->SetIsEnabled(true);
        }
    }
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
        FString pingString { FString::FromInt(FMath::RoundToInt(this->AveragePlayerLatency)) + " ms" };
        if (!ensure(this->PingTextBlock != nullptr)) return;
        this->PingTextBlock->SetText(FText::FromString(pingString));
    }
}

void UMainMenuWidget::onStartMatchmakingResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful)
{
    this->MatchmakingButton->SetIsEnabled(true);

    if (!_bWasSuccessful)
    {
        return;
    }

    TSharedPtr<FJsonObject> jsonObject;
    TSharedRef<TJsonReader<>> reader { TJsonReaderFactory<>::Create(_response->GetContentAsString()) };
    if (!FJsonSerializer::Deserialize(reader, jsonObject)
        || !jsonObject->HasField("ticketId"))
    {
        return;
    }

    FString matchmakingTicketId { jsonObject->GetStringField("ticketId") };

    UHub_SpacelGameInstance* spacelGameInstance { Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
    if (!ensure(spacelGameInstance != nullptr)) return;

    spacelGameInstance->MatchmakingTicketId = matchmakingTicketId;

    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    world->GetTimerManager().SetTimer(this->PollMatchmakingHandle, this, &UMainMenuWidget::PollMatchmaking, 1.0f, true, 1.0f);
    this->SearchingForGame = true;

    setMatchkingTextBlock(FText::FromString("Cancel Matchmaking"), FText::FromString("Currently looking for a match"));
}

void UMainMenuWidget::onStopMatchmakingResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful)
{
    UHub_SpacelGameInstance* spacelGameInstance { Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
    if (!ensure(spacelGameInstance != nullptr)) return;

    spacelGameInstance->MatchmakingTicketId = {};

    setMatchkingTextBlock(FText::FromString("Join Game"), FText::FromString(""));

    this->MatchmakingButton->SetIsEnabled(true);
}

void UMainMenuWidget::PollMatchmaking()
{
    UHub_SpacelGameInstance* spacelGameInstance { Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
    if (!ensure(spacelGameInstance != nullptr)) return;

    FString accessToken { spacelGameInstance->AccessToken },
            matchmakingTicketId { spacelGameInstance->MatchmakingTicketId };

    if (accessToken.Len() > 0 && matchmakingTicketId.Len() > 0 && this->SearchingForGame)
    {
        TSharedPtr<FJsonObject> requestObj { MakeShareable(new FJsonObject) };
        requestObj->SetStringField("ticketId", matchmakingTicketId);

        FString requestBody {};
        TSharedRef<TJsonWriter<>> writer { TJsonWriterFactory<>::Create(&requestBody) };
        if (FJsonSerializer::Serialize(requestObj.ToSharedRef(), writer))
        {
            SimplyHttpRequest::processRequest(this->HttpModule, this,
                &UMainMenuWidget::onPollMatchmakingReceived,
                this->ApiUrl + "/pollmatchmaking", "POST",
                TArray<FString>{"Content-Type", "application/json", "Authorization", accessToken}, requestBody);
        }
    }
}

void UMainMenuWidget::onPollMatchmakingReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful)
{
    if (!_bWasSuccessful || !this->SearchingForGame)
    {
        return;
    }

    TSharedPtr<FJsonObject> jsonObject;
    TSharedRef<TJsonReader<>> reader { TJsonReaderFactory<>::Create(_response->GetContentAsString()) };
    if (!FJsonSerializer::Deserialize(reader, jsonObject)
        || !jsonObject->HasField("ticket"))
    {
        return;
    }

    TSharedPtr<FJsonObject> ticket { jsonObject->GetObjectField("ticket") };
    FString ticketType { ticket->GetObjectField("Type")->GetStringField("S") };

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

            TSharedPtr<FJsonObject> gameSessionInfo { ticket->GetObjectField("GameSessionInfo")->GetObjectField("M") };
            FString ipAddress { gameSessionInfo->GetObjectField("IpAddress")->GetStringField("S") },
                    port { gameSessionInfo->GetObjectField("Port")->GetStringField("N") };

            TArray<TSharedPtr<FJsonValue>> players { ticket->GetObjectField("Players")->GetArrayField("L") };
            TSharedPtr<FJsonObject> player { players[0]->AsObject()->GetObjectField("M") };
            FString playerSessionId { player->GetObjectField("PlayerSessionId")->GetStringField("S") },
                    playerId { player->GetObjectField("PlayerId")->GetStringField("S") };

            FString levelName { ipAddress + ":" + port };
            FString const& options { "?PlayerSessionId=" + playerSessionId + "?PlayerId=" + playerId };
            //UE_LOG(LogTemp, Warning, TEXT("options : %s"), *options);

            // save player name
            if (USpacelSaveGame* saveGameInstance = Cast<USpacelSaveGame>(UGameplayStatics::CreateSaveGameObject(USpacelSaveGame::StaticClass())))
            {
                // Set data on the savegame object.
                saveGameInstance->PlayerName = spacelGameInstance->CustomPlayerName;

                // Start async save process.
                UGameplayStatics::AsyncSaveGameToSlot(saveGameInstance, "Save", 0);
            }

            UGameplayStatics::OpenLevel(world, FName(*levelName), false, options);
        }
        else
        {
            setMatchkingTextBlock(FText::FromString("Join Game"), FText::FromString(ticketType + ". Please try again"));
        }
    }
}

void UMainMenuWidget::setMatchkingTextBlock(TOptional<FText>&& _button, TOptional<FText>&& _event)
{
    if (_button.IsSet())
    {
        UTextBlock* buttonTextBlock { (UTextBlock*)this->MatchmakingButton->GetChildAt(0) };
        if (buttonTextBlock != nullptr)
        {
            buttonTextBlock->SetText(_button.GetValue());
        }
    }
    if (_event.IsSet())
    {
        if (this->MatchmakingEventTextBlock != nullptr)
        {
            this->MatchmakingEventTextBlock->SetText(_event.GetValue());
        }
    }
}

void UMainMenuWidget::OnPlayerNameChange(FText const& _text)
{
    UHub_SpacelGameInstance* spacelGameInstance = Cast<UHub_SpacelGameInstance>(this->GetGameInstance());
    if (!ensure(spacelGameInstance != nullptr)) return;
    // set player name
    spacelGameInstance->CustomPlayerName = _text.ToString();
}