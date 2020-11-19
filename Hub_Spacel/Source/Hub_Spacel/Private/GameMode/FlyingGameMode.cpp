// Fill out your copyright notice in the Description page of Project Settings.


#include "FlyingGameMode.h"
#include "Player/SpacelPlayerState.h"
#include "GameState/SpacelGameState.h"
#include "TextReaderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Util/SimplyHttpRequest.h"

AFlyingGameMode::AFlyingGameMode()
{
    UTextReaderComponent* textReader = CreateDefaultSubobject<UTextReaderComponent>(TEXT("TextReaderComponent"));
    if (!ensure(textReader != nullptr)) return;
    ApiUrl = textReader->ReadFile("Urls/ApiUrl.txt");

    HttpModule = &FHttpModule::Get();

    RemainingGameTime = 10 * 60;
    GameSessionActivated = false;
}

void AFlyingGameMode::BeginPlay()
{
    Super::BeginPlay();

#if WITH_GAMELIFT
    auto initSDKOutCome = Aws::GameLift::Server::InitSDK();

    if (initSDKOutCome.IsSuccess())
    {
        auto onStartGameSession = [](Aws::GameLift::Server::Model::GameSession _gameSessionObj, void* _params)
        {
            FStartGameSessionState* state = (FStartGameSessionState*)_params;
            if (!ensure(state != nullptr)) return;
            state->Status = Aws::GameLift::Server::ActivateGameSession().IsSuccess();

            FString matchmakerData { _gameSessionObj.GetMatchmakerData() };

            TSharedPtr<FJsonObject> jsonObject {};
            TSharedRef<TJsonReader<>> reader { TJsonReaderFactory<>::Create(matchmakerData) };

            if (FJsonSerializer::Deserialize(reader, jsonObject))
            {
                state->MatchmakingConfigurationArn = jsonObject->GetStringField("matchmakingConfigurationArn");

                TArray<TSharedPtr<FJsonValue>> teams { jsonObject->GetArrayField("teams") };
                for(TSharedPtr<FJsonValue> team : teams)
                {
                    TSharedPtr<FJsonObject> teamObj { team->AsObject() };
                    FString teamName { teamObj->GetStringField("name") };

                    TArray<TSharedPtr<FJsonValue>> players { teamObj->GetArrayField("players") };

                    for(TSharedPtr<FJsonValue> player : players)
                    {
                        TSharedPtr<FJsonObject> playerObj { player->AsObject() };
                        FString playerId { playerObj->GetStringField("playerId") };

                        TSharedPtr<FJsonObject> attributes { playerObj->GetObjectField("attributes") };
                        TSharedPtr<FJsonObject> skill { attributes->GetObjectField("skill") };

                        FString skillValue { skill->GetStringField("valueAttribute") };

                        auto skillAttributeValue { new Aws::GameLift::Server::Model::AttributeValue(FCString::Atod(*skillValue)) };

                        Aws::GameLift::Server::Model::Player awsPlayerObj;
                        awsPlayerObj.SetPlayerId(TCHAR_TO_ANSI(*playerId));
                        awsPlayerObj.SetTeam(TCHAR_TO_ANSI(*teamName));
                        awsPlayerObj.AddPlayerAttribute("skill", *skillAttributeValue);

                        state->PlayerIdToPlayer.Add(playerId, awsPlayerObj);
                    }
                }
            }
        };

        auto onUpdateGameSession = [](Aws::GameLift::Server::Model::UpdateGameSession _updateGameSessionObj, void* _params)
        {
            FUpdateGameSessionState* state = (FUpdateGameSessionState*)_params;
            if (!ensure(state != nullptr)) return;
        };

        auto onProcessTerminate = [](void* _params)
        {
            FProcessTerminateState* state = (FProcessTerminateState*)_params;
            if (!ensure(state != nullptr)) return;
            auto getTerminationTimeOutcome = Aws::GameLift::Server::GetTerminationTime();
            if(getTerminationTimeOutcome.IsSuccess())
            {
                state->m_terminationTime = getTerminationTimeOutcome.GetResult();
            }

            state->Status = true;
        };

        auto onHealthCheck = [](void* _params) -> bool
        {
            FHealthCheckState* state = (FHealthCheckState*)_params;
            if (!ensure(state != nullptr)) return false;
            state->Status = true;

            return state->Status;
        };

        TArray<FString> commandLineTokens;
        TArray<FString> commandLineSwitches;
        int port = FURL::UrlConfig.DefaultPort;

        // Spacel.exe token -port=7777
        FCommandLine::Parse(FCommandLine::Get(), commandLineTokens, commandLineSwitches);

        for(FString str : commandLineSwitches)
        {
            FString key {}, value {};

            if(str.Split("=", &key, &value))
            {
                if(key.Equals("port"))
                {
                    port = FCString::Atoi(*value);
                }
                else if(key.Equals("password"))
                {
                    this->ServerPassword = value;
                }
            }
        }

        const char * logFile = "aLogFile.txt";
        const char ** logFiles = &logFile;
        auto logParams = new Aws::GameLift::Server::LogParameters(logFiles, 1);

        auto params = new Aws::GameLift::Server::ProcessParameters(
            onStartGameSession,
            &StartGameSessionState,
            onUpdateGameSession,
            &UpdateGameSessionState,
            onProcessTerminate,
            &ProcessTerminateState,
            onHealthCheck,
            &HealthCheckState,
            port,
            *logParams
        );

        auto processReadyOutcome = Aws::GameLift::Server::ProcessReady(*params);
        if (!ensure(processReadyOutcome.IsSuccess())) return;
    }
#endif
    this->GetWorldTimerManager().SetTimer(this->HandleGameSessionUpdateHandle, this, &AFlyingGameMode::HandleGameSessionUpdate, 1.0f, true, 5.0f);
    this->GetWorldTimerManager().SetTimer(this->HandleProcessTerminationHandle, this, &AFlyingGameMode::HandleProcessTermination, 1.0f, true, 5.0f);

    // TO DO : FOR TEST
    /*ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(this->GameState);
    if (!ensure(spacelGameState != nullptr)) return;
    spacelGameState->LatestEvent = "GameEnded";
    spacelGameState->WinningTeam = "Team 1";*/
}

void AFlyingGameMode::PreLogin(FString const& _options, FString const& _address, FUniqueNetIdRepl const& _uniqueId, FString& _errorMessage)
{
    Super::PreLogin(_options, _address, _uniqueId, _errorMessage);

#if WITH_GAMELIFT
    if (_options.Len() > 0)
    {
        FString const& playerSessionId { UGameplayStatics::ParseOption(_options, "PlayerSessionId") };
        FString const& playerId { UGameplayStatics::ParseOption(_options, "PlayerId") };

        if (playerSessionId.Len() > 0 && playerId.Len() > 0)
        {
            Aws::GameLift::Server::Model::DescribePlayerSessionsRequest describePlayerSessionsRequest;
            describePlayerSessionsRequest.SetPlayerSessionId(TCHAR_TO_ANSI(*playerSessionId));

            auto describePlayerSessionsOutcome { Aws::GameLift::Server::DescribePlayerSessions(describePlayerSessionsRequest) };
            if (describePlayerSessionsOutcome.IsSuccess())
            {
                auto describePlayerSessionsResult { describePlayerSessionsOutcome.GetResult() };
                int count { 1 };
                auto playerSessions { describePlayerSessionsResult.GetPlayerSessions(count) };
                if (playerSessions != nullptr)
                {
                    auto playerSession { playerSessions[0] };
                    FString expectedPlayerId { playerSession.GetPlayerId() };
                    auto playerStatus { playerSession.GetStatus() };

                    if (expectedPlayerId.Equals(playerId)
                        && playerStatus == Aws::GameLift::Server::Model::PlayerSessionStatus::RESERVED)
                    {
                        auto acceptPlayerSessionOutCome = Aws::GameLift::Server::AcceptPlayerSession(TCHAR_TO_ANSI(*playerSessionId));

                        if (!acceptPlayerSessionOutCome.IsSuccess())
                        {
                            _errorMessage = "Unauthorized player reject";
                        }
                    }
                    else
                    {
                        _errorMessage = "Unauthorized expectedPlayerId or playerStatus are wrong";
                    }
                }
                else
                {
                    _errorMessage = "Unauthorized no player sessions";
                }
            }
            else
            {
                _errorMessage = "Unauthorized describePlayerSessionsOutcome fail";
            }
        }
        else
        {
            _errorMessage = "Unauthorized no player session id or player id";
        }
    }
    else
    {
        _errorMessage = "Unauthorized no options";
    }
#endif
}

void AFlyingGameMode::Logout(class AController* _exiting)
{
#if WITH_GAMELIFT
    if (_exiting != nullptr)
    {
        ASpacelPlayerState* spacelPlayerState { Cast<ASpacelPlayerState>(_exiting->PlayerState) };
        if (spacelPlayerState != nullptr)
        {
            FString const& playerSessionId { spacelPlayerState->PlayerSessionId };
            if (playerSessionId.Len() > 0)
            {
                Aws::GameLift::Server::RemovePlayerSession(TCHAR_TO_ANSI(*playerSessionId));
            }
        }
    }
#endif
    Super::Logout(_exiting);
}

FString AFlyingGameMode::InitNewPlayer(class APlayerController* _newPlayerController, FUniqueNetIdRepl const& _uniqueId, FString const& _options, FString const& _portal)
{
    FString initializedString { Super::InitNewPlayer(_newPlayerController, _uniqueId, _options, _portal) };

    /*if (_newPlayerController != nullptr)
    {
        ASpacelPlayerState* playerState = Cast<ASpacelPlayerState>(_newPlayerController->PlayerState);
        if (playerState != nullptr)
        {
            // TO DO : attribute right team
            FMath::RandRange(0, 1) == 0 ? playerState->Team = "Team 1" : playerState->Team = "Team 2";
        }
    }*/

#if WITH_GAMELIFT
    FString const& playerSessionId { UGameplayStatics::ParseOption(_options, "PlayerSessionId") };
    FString const& playerId { UGameplayStatics::ParseOption(_options, "PlayerId") };

    if (_newPlayerController != nullptr)
    {
        ASpacelPlayerState* spacelPlayerState { Cast<ASpacelPlayerState>(_newPlayerController->PlayerState) };
        if (spacelPlayerState != nullptr)
        {
            spacelPlayerState->PlayerSessionId = *playerSessionId;
            spacelPlayerState->MatchmakingPlayerId = *playerId;

            if (this->StartGameSessionState.PlayerIdToPlayer.Num() > 0
             && this->StartGameSessionState.PlayerIdToPlayer.Contains(playerId))
            {
                auto playerObj { this->StartGameSessionState.PlayerIdToPlayer.Find(playerId) };
                FString const& team { playerObj->GetTeam() };
                spacelPlayerState->Team = *team;
            }
        }
    }
#endif
    return initializedString;
}

void AFlyingGameMode::CountDownUntilGameOver()
{
    ASpacelGameState* spacelGameState { Cast<ASpacelGameState>(this->GameState) };
    if (!ensure(spacelGameState != nullptr)) return;
    spacelGameState->LatestEvent = FString::FromInt(this->RemainingGameTime) + " seconds until game is over";

    if (this->RemainingGameTime > 0)
    {
        this->RemainingGameTime--;
    }
    else
    {
        GetWorldTimerManager().ClearTimer(this->CountDownUntilGameOverHandle);
    }
}

void AFlyingGameMode::EndGame()
{
    GetWorldTimerManager().ClearTimer(this->CountDownUntilGameOverHandle);
    GetWorldTimerManager().ClearTimer(this->EndGameHandle);
    GetWorldTimerManager().ClearTimer(this->PickAWinningTeamHandle);
    GetWorldTimerManager().ClearTimer(this->HandleProcessTerminationHandle);
    GetWorldTimerManager().ClearTimer(this->HandleGameSessionUpdateHandle);

#if WITH_GAMELIFT
    Aws::GameLift::Server::TerminateGameSession();
    Aws::GameLift::Server::ProcessEnding();
    FGenericPlatformMisc::RequestExit(false);
#endif
}

void AFlyingGameMode::PickAWinningTeam()
{
    GetWorldTimerManager().ClearTimer(this->CountDownUntilGameOverHandle);

#if WITH_GAMELIFT
    ASpacelGameState* spacelGameState { Cast<ASpacelGameState>(this->GameState) };
    if (spacelGameState != nullptr)
    {
        spacelGameState->LatestEvent = "GameEnded";

        // TO DO Make rules winning team
        FMath::RandRange(0, 1) == 0 ? spacelGameState->WinningTeam = "Team 1" : spacelGameState->WinningTeam = "Team 2";

        TSharedPtr<FJsonObject> requestObj { MakeShareable(new FJsonObject) };
        requestObj->SetStringField("winningTeam", spacelGameState->WinningTeam);

        auto getGameSessionIdOutcome { Aws::GameLift::Server::GetGameSessionId() };
        if (getGameSessionIdOutcome.IsSuccess())
        {
            requestObj->SetStringField("gameSessionId", getGameSessionIdOutcome.GetResult());

            FString requestBody {};
            TSharedRef<TJsonWriter<>> writer { TJsonWriterFactory<>::Create(&requestBody) };
            if (FJsonSerializer::Serialize(requestObj.ToSharedRef(), writer))
            {
                SimplyHttpRequest::processRequest(this->HttpModule, this, 
                    &AFlyingGameMode::onRecordMatchResultResponseReceive,
                    this->ApiUrl + "/recordmatchresult", "POST",
                    TArray<FString> {"Authorization", this->ServerPassword, "Content-Type", "application/json"},
                    requestBody);
                // early return, no need to set timer in this case (and the only case)
                return;
            }
        }
    }
    // something wrong
    GetWorldTimerManager().SetTimer(this->EndGameHandle, this, &AFlyingGameMode::EndGame, 1.0f, false, 5.0f);
#endif
}

void AFlyingGameMode::HandleProcessTermination()
{
    if (this->ProcessTerminateState.Status)
    {
        GetWorldTimerManager().ClearTimer(this->CountDownUntilGameOverHandle);
        GetWorldTimerManager().ClearTimer(this->HandleProcessTerminationHandle);
        GetWorldTimerManager().ClearTimer(this->HandleGameSessionUpdateHandle);

        FString processInterruptionMessage {};

        if (this->ProcessTerminateState.m_terminationTime <= 0L)
        {
            processInterruptionMessage = "Server process could shut down at any time";
        }
        else
        {
            long timeLeft { (long)(this->ProcessTerminateState.m_terminationTime - FDateTime::Now().ToUnixTimestamp()) };
            processInterruptionMessage = FString::Printf(TEXT("Server process scheduled to terminate in %ld seconds"), timeLeft);
        }

        ASpacelGameState* spacelGameState { Cast<ASpacelGameState>(this->GameState) };
        if (spacelGameState != nullptr)
        {
            spacelGameState->LatestEvent = processInterruptionMessage;
        }

        GetWorldTimerManager().SetTimer(this->EndGameHandle, this, &AFlyingGameMode::EndGame, 1.0f, false, 10.0f);
    }
}

void AFlyingGameMode::HandleGameSessionUpdate()
{
    if (!this->GameSessionActivated && this->StartGameSessionState.Status)
    {
        this->GameSessionActivated = true;

        GetWorldTimerManager().SetTimer(this->PickAWinningTeamHandle, this, &AFlyingGameMode::PickAWinningTeam, 1.0f, false, (float)this->RemainingGameTime);
        GetWorldTimerManager().SetTimer(this->CountDownUntilGameOverHandle, this, &AFlyingGameMode::CountDownUntilGameOver, 1.0f, true, 0.0f);
    }
}

void AFlyingGameMode::onRecordMatchResultResponseReceive(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful)
{
    this->GetWorldTimerManager().SetTimer(this->EndGameHandle, this, &AFlyingGameMode::EndGame, 1.0f, false, 5.0f);
}