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
#include "Player/Save/SpacelSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetStringLibrary.h"
#include "DataAsset/HideSkillDataAsset.h"
#include "DataAsset/SkillDataAsset.h"
#include "DataAsset/UniqueSkillDataAsset.h"
#include "GameFramework/PlayerInput.h"
#include "GameFramework/InputSettings.h"

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

    if (auto* gameMode = UGameplayStatics::GetGameMode(this->GetWorld()))
    {
        if (gameMode->OptionsString.Contains(Option::Requeue))
        {
            OnPlay();
        }
    }
}

void UGlobalMainMenu::NativeDestruct()
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    world->GetTimerManager().ClearTimer(this->PollMatchmakingHandle);
    world->GetTimerManager().ClearTimer(this->SetAveragePlayerLatencyHandle);
}

void UGlobalMainMenu::OnLoadGame(FString const& _playerName)
{
    m_playerProfile.m_playerName = _playerName;
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

void UGlobalMainMenu::PollMatchmaking()
{
    UHub_SpacelGameInstance* spacelGameInstance{ Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
    if (!ensure(spacelGameInstance != nullptr)) return;

    FString accessToken{ spacelGameInstance->AccessToken },
        matchmakingTicketId{ spacelGameInstance->MatchmakingTicketId };

    if (accessToken.Len() > 0 && matchmakingTicketId.Len() > 0 && this->SearchingForGame)
    {
        TSharedPtr<FJsonObject> requestObj{ MakeShareable(new FJsonObject) };
        requestObj->SetStringField("ticketId", matchmakingTicketId);

        FString requestBody{};
        TSharedRef<TJsonWriter<>> writer{ TJsonWriterFactory<>::Create(&requestBody) };
        if (FJsonSerializer::Serialize(requestObj.ToSharedRef(), writer))
        {
            SimplyHttpRequest::processRequest(this->HttpModule, this,
                &UGlobalMainMenu::onPollMatchmakingReceived,
                this->ApiUrl + "/pollmatchmaking", "POST",
                TArray<FString>{"Content-Type", "application/json", "Authorization", accessToken}, requestBody);
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

    m_playerProfile.m_win = playerData->GetObjectField("Wins")->GetStringField("N") + " win(s)";
    FString const& lostStr = playerData->GetObjectField("Losses")->GetStringField("N");

    int win = UKismetStringLibrary::Conv_StringToInt(m_playerProfile.m_win);
    int lost = UKismetStringLibrary::Conv_StringToInt(lostStr);

    m_playerProfile.m_ratio = "Win ratio " + FString::FromInt( ((float)win / (float)(lost + win)) * 100 ) + "%";
    m_playerProfile.m_nbGame = FString::FromInt(win + lost) + " game played";
    SimplyUI::setVisibility({ ESlateVisibility::Hidden },
        std::make_tuple(this->WebBrowser));
}

void UGlobalMainMenu::SetPlayerProfile(FString& _playerName, FString& _win, FString& _ratio, FString& _nbGame)
{
    _playerName = m_playerProfile.m_playerName;
    _win = m_playerProfile.m_win;
    _ratio = m_playerProfile.m_ratio;
    _nbGame = m_playerProfile.m_nbGame;
}

void UGlobalMainMenu::RefreshPlayerName()
{
    UHub_SpacelGameInstance* spacelGameInstance{ Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
    if (!ensure(spacelGameInstance != nullptr)) return;

    m_playerProfile.m_playerName = spacelGameInstance->CustomPlayerName;
}

void UGlobalMainMenu::onStartMatchmakingResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful)
{
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

    world->GetTimerManager().SetTimer(this->PollMatchmakingHandle, this, &UGlobalMainMenu::PollMatchmaking, 1.0f, true, 1.0f);

    BP_MatchmakingState("Search an epic battle");
}

void UGlobalMainMenu::onStopMatchmakingResponseReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful)
{
    UHub_SpacelGameInstance* spacelGameInstance{ Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
    if (!ensure(spacelGameInstance != nullptr)) return;

    spacelGameInstance->MatchmakingTicketId = {};

    BP_MatchmakingState("Join Game");
    BP_Timer(false);
    this->SearchingForGame = false;
}

void UGlobalMainMenu::onPollMatchmakingReceived(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful)
{
    if (!_bWasSuccessful || !this->SearchingForGame)
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
            BP_MatchmakingState("Let's go !");

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
            BP_MatchmakingState("Join Game");
            BP_Timer(false);
        }
    }
}

bool UGlobalMainMenu::OnPlay()
{
    if (this->SearchingForGame)
    {
        cancelSearch();
    }
    else
    {
        startSearch();
    }

    return this->SearchingForGame;
}

void UGlobalMainMenu::OnRefreshInput(class UInputSettings* _inputSettings)
{
    if (this->SkillDataAsset == nullptr) return;
    if (this->HideSkillDataAsset == nullptr) return;

    USkillDataAsset* skillDataAsset = this->SkillDataAsset;
    UHideSkillDataAsset* hideSkillDataAsset = this->HideSkillDataAsset;

    auto lbsetAction = [&_inputSettings, &skillDataAsset, &hideSkillDataAsset](FName&& _actionName, EInput _type)
    {
        if (FHideSkill* hideSkill = hideSkillDataAsset->get(_type))
        {
            TArray<FInputActionKeyMapping> _outMappings;
            _inputSettings->GetActionMappingByName(_actionName, _outMappings);
            for (auto const& mapping : _outMappings)
            {
                hideSkill->Key = mapping.Key;

                auto const& skills = skillDataAsset->getSkillByInput(_type);
                for (auto* skill : skills)
                {
                    if (skill != nullptr)
                    {
                        skill->Key = mapping.Key;
                    }
                }

                break;
            }
        }
    };

    auto lbsetAxis = [&_inputSettings, &skillDataAsset, &hideSkillDataAsset](FName&& _axisName, EInput _type)
    {
        if (FHideSkill* hideSkill = hideSkillDataAsset->get(_type))
        {
            TArray<FInputAxisKeyMapping> _outMappings;
            _inputSettings->GetAxisMappingByName(_axisName, _outMappings);
            for (auto const& mapping : _outMappings)
            {
                hideSkill->Key = mapping.Key;
            }
        }
    };

    lbsetAction("EscapeMode", EInput::EscapeMode);
    lbsetAction("Heal", EInput::HealSkill);
    lbsetAction("SecondaryShot", EInput::SecondaryShot);
    lbsetAction("RepairProtection", EInput::RepairProtection);
    lbsetAction("RepairEngine", EInput::RepairEngine);
    lbsetAction("Emergency", EInput::Emergency);
    lbsetAction("MediumSkill", EInput::MediumSkill);
    lbsetAction("HightSkill", EInput::HightSkill);

    lbsetAxis("Forward", EInput::Forward);
    lbsetAxis("Backward", EInput::Backward);
    lbsetAxis("FlightAttitudeLeft", EInput::FlightAltitudeLeft);
    lbsetAxis("FlightAttitudeRight", EInput::FlightAltitudeRight);
    lbsetAxis("HorizontalStrafRight", EInput::Right);
    lbsetAxis("HorizontalStrafLeft", EInput::Left);

    // special case for HalfTurn
    TArray<FInputAxisKeyMapping> outMappingsBackward;
    _inputSettings->GetAxisMappingByName("Backward", outMappingsBackward);

    for (auto const& mapping : outMappingsBackward)
    {
        TArray<FInputActionKeyMapping> outMappingsHalfTurn;
        _inputSettings->GetActionMappingByName("HalfTurn", outMappingsHalfTurn);

        for (auto const& halfTurnMapping : outMappingsHalfTurn)
        {
            _inputSettings->RemoveActionMapping(halfTurnMapping, false);
        }

        FInputActionKeyMapping newKeyMapping = FInputActionKeyMapping("HalfTurn", mapping.Key);
        _inputSettings->AddActionMapping(newKeyMapping, true);
    }
}

void UGlobalMainMenu::startSearch()
{
    UHub_SpacelGameInstance* spacelGameInstance{ Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
    if (!ensure(spacelGameInstance != nullptr)) return;

    FString accessToken{ spacelGameInstance->AccessToken };
    FString matchmakingTicketId{ spacelGameInstance->MatchmakingTicketId };

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

            BP_MatchmakingState("Start matchmaking");
            BP_Timer(true);
            this->SearchingForGame = true;
        }
    }
}

void UGlobalMainMenu::cancelSearch()
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    world->GetTimerManager().ClearTimer(this->PollMatchmakingHandle);

    UHub_SpacelGameInstance* spacelGameInstance{ Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
    if (!ensure(spacelGameInstance != nullptr)) return;

    FString accessToken { spacelGameInstance->AccessToken };
    FString matchmakingTicketId { spacelGameInstance->MatchmakingTicketId };

    if (accessToken.Len() > 0 && matchmakingTicketId.Len() > 0)
    {
        TSharedPtr<FJsonObject> requestObj{ MakeShareable(new FJsonObject) };
        requestObj->SetStringField("ticketId", matchmakingTicketId);

        FString requestBody;
        TSharedRef<TJsonWriter<>> writer = TJsonWriterFactory<>::Create(&requestBody);
        if (FJsonSerializer::Serialize(requestObj.ToSharedRef(), writer))
        {
            SimplyHttpRequest::processRequest(this->HttpModule, this,
                &UGlobalMainMenu::onStopMatchmakingResponseReceived,
                this->ApiUrl + "/stopmatchmaking", "POST",
                TArray<FString>{"Content-Type", "application/json", "Authorization", accessToken},
                requestBody);
        }
    }
}