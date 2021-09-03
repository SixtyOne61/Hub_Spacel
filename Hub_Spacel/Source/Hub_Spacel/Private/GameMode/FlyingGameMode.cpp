// Fill out your copyright notice in the Description page of Project Settings.


#include "FlyingGameMode.h"
#include "Player/SpacelPlayerState.h"
#include "GameState/SpacelGameState.h"
#include "GameFramework/PlayerStart.h"
#include "TextReaderComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Json.h"
#include "JsonUtilities.h"
#include "Util/SimplyHttpRequest.h"
#include "Util/SimplyMath.h"
#include "DataAsset/FlyingGameModeDataAsset.h"
#include "DataAsset/EditorHackDataAsset.h"

AFlyingGameMode::AFlyingGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    UTextReaderComponent* textReader = CreateDefaultSubobject<UTextReaderComponent>(TEXT("TextReaderComponent"));
    if (!ensure(textReader != nullptr)) return;
    ApiUrl = textReader->ReadFile("Urls/ApiUrl.txt");

    HttpModule = &FHttpModule::Get();
}

void AFlyingGameMode::BeginPlay()
{
    Super::BeginPlay();

    if(this->GameModeDataAsset != nullptr)
    {
        this->RemainingGameTime = this->GameModeDataAsset->RemainingGameTime;
        this->RemainingChooseModuleTime = this->GameModeDataAsset->RemainingChooseModuleTime;
#if WITH_EDITOR
        if (HackDataAsset != nullptr && HackDataAsset->UseHack)
        {
            RemainingGameTime = HackDataAsset->RemainingGameTime;
            RemainingChooseModuleTime = HackDataAsset->ChooseModuleTime;
        }
#endif

        this->RemainingLeaveTime = this->GameModeDataAsset->RemainingLeaveTime;
    }

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

            auto reason { _updateGameSessionObj.GetUpdateReason() };

            switch(reason)
            {
                case Aws::GameLift::Server::Model::UpdateReason::MATCHMAKING_DATA_UPDATED:
                {
                    state->Reason = EUpdateReason::MATCHMAKING_DATA_UPDATED;

                    auto gameSessionObj = _updateGameSessionObj.GetGameSession();
                    FString matchmakerData{ gameSessionObj.GetMatchmakerData() };

                    TSharedPtr<FJsonObject> jsonObject{};
                    TSharedRef<TJsonReader<>> reader{ TJsonReaderFactory<>::Create(matchmakerData) };

                    if (FJsonSerializer::Deserialize(reader, jsonObject))
                    {
                        TArray<TSharedPtr<FJsonValue>> teams{ jsonObject->GetArrayField("teams") };
                        for (TSharedPtr<FJsonValue> team : teams)
                        {
                            TSharedPtr<FJsonObject> teamObj{ team->AsObject() };
                            FString teamName{ teamObj->GetStringField("name") };

                            TArray<TSharedPtr<FJsonValue>> players{ teamObj->GetArrayField("players") };

                            for (TSharedPtr<FJsonValue> player : players)
                            {
                                TSharedPtr<FJsonObject> playerObj{ player->AsObject() };
                                FString playerId{ playerObj->GetStringField("playerId") };

                                TSharedPtr<FJsonObject> attributes{ playerObj->GetObjectField("attributes") };
                                TSharedPtr<FJsonObject> skill{ attributes->GetObjectField("skill") };

                                FString skillValue{ skill->GetStringField("valueAttribute") };

                                auto skillAttributeValue{ new Aws::GameLift::Server::Model::AttributeValue(FCString::Atod(*skillValue)) };

                                Aws::GameLift::Server::Model::Player awsPlayerObj;
                                awsPlayerObj.SetPlayerId(TCHAR_TO_ANSI(*playerId));
                                awsPlayerObj.SetTeam(TCHAR_TO_ANSI(*teamName));
                                awsPlayerObj.AddPlayerAttribute("skill", *skillAttributeValue);

                                state->PlayerIdToPlayer.Add(playerId, awsPlayerObj);
                            }
                        }
                    }
                }
                break;

                case Aws::GameLift::Server::Model::UpdateReason::BACKFILL_CANCELLED:
                    state->Reason = EUpdateReason::BACKFILL_CANCELLED;
                break;

                case Aws::GameLift::Server::Model::UpdateReason::BACKFILL_FAILED:
                    state->Reason = EUpdateReason::BACKFILL_FAILED;
                break;

                case Aws::GameLift::Server::Model::UpdateReason::BACKFILL_TIMED_OUT:
                    state->Reason = EUpdateReason::BACKFILL_TIMED_OUT;
                break;

                default:
                    state->Reason = EUpdateReason::NO_UPDATE_RECEIVED;
                break;
            }
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

#if WITH_EDITOR
    ASpacelGameState* spacelGameState{ Cast<ASpacelGameState>(this->GameState) };
    if (!ensure(spacelGameState != nullptr)) return;
    spacelGameState->GoToPrepare();
    m_timerSeconde = this->RemainingChooseModuleTime;
#endif

    TArray<AActor*> out {};
    UGameplayStatics::GetAllActorsOfClass(this->GetWorld(), APlayerStart::StaticClass(), out);
    for(AActor* act : out)
    {
        if(act == nullptr || act->IsPendingKill()) continue;

        if(act->Tags.Num() == 0) continue;

        FName teamName { act->Tags[0] };
        if(!m_startLocation.Contains(teamName))
        {
            m_startLocation.Add(teamName);
        }

        m_startLocation[teamName].Add({false, act->GetTransform()});
    }
}

void AFlyingGameMode::Tick(float _deltaSeconde)
{
    Super::Tick(_deltaSeconde);

    if(m_timerSeconde > 0.0f)
    {
        m_timerSeconde = FMath::Max(m_timerSeconde - _deltaSeconde, 0.0f);
    }
    else
    {
        if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(this->GameState))
        {
            EGameState state = spacelGameState->GetState();
            switch(state)
            {
                case EGameState::Prepare:
                {
                    spacelGameState->GoToLockLowModule();
                    // register all team for scoring
                    spacelGameState->RegisterTeam();
                    m_timerSeconde = this->RemainingChooseModuleTime;
                    break;
                }

                case EGameState::LockLowModule:
                {
                    spacelGameState->GoToLockMediumModule();
                    m_timerSeconde = this->RemainingChooseModuleTime;
                    break;
                }

                case EGameState::LockMediumModule:
                {
                    spacelGameState->GoToLockPrepare();
                    if (this->GameModeDataAsset != nullptr)
                    {
                        this->RemainingChooseModuleTime = this->GameModeDataAsset->EndModuleTime;
                    }
                    m_timerSeconde = this->RemainingChooseModuleTime;
                    break;
                }

                case EGameState::LockPrepare:
                {
                    EndLobby();
                    break;
                }

                case EGameState::InGame:
                {
                    this->PickAWinningTeam();
                    break;
                }

                case EGameState::EndGame:
                {
                    this->LeaveLevel();
                    break;
                }

                default:
                break;
            }
        }
    }
}

FTransform AFlyingGameMode::GetSpawnLocation(FName const& _team)
{
    if(!m_startLocation.Contains(_team))
    {
        return {};
    }

    for(FStartLocation & startLocation : m_startLocation[_team])
    {
        if(!startLocation.m_isUse)
        {
            startLocation.m_isUse = true;
            startLocation.m_transform.SetRotation(
                SimplyMath::MyLookRotation(FVector::ZeroVector, startLocation.m_transform.GetRotation().GetUpVector(), startLocation.m_transform.GetLocation()).Quaternion()
                );
            return startLocation.m_transform;
        }
    }

    return {};
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
    if (this->LatestBackfillTicketId.Len() > 0)
    {
        auto gameSessionIdOutcome = Aws::GameLift::Server::GetGameSessionId();
        if (gameSessionIdOutcome.IsSuccess())
        {
            FString gameSessionId{ gameSessionIdOutcome.GetResult() };
            FString matchmakingConfigurationArn{ this->StartGameSessionState.MatchmakingConfigurationArn };
            stopBackfillRequest(gameSessionId, matchmakingConfigurationArn, this->LatestBackfillTicketId);
        }
    }

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

            if (this->UpdateGameSessionState.PlayerIdToPlayer.Num() > 0
                && this->UpdateGameSessionState.PlayerIdToPlayer.Contains(playerId))
            {
                auto playerObj{ this->UpdateGameSessionState.PlayerIdToPlayer.Find(playerId) };
                FString const& team{ playerObj->GetTeam() };
                spacelPlayerState->SetTeam(*team);
            }
            else if (this->StartGameSessionState.PlayerIdToPlayer.Num() > 0
             && this->StartGameSessionState.PlayerIdToPlayer.Contains(playerId))
            {
                auto playerObj { this->StartGameSessionState.PlayerIdToPlayer.Find(playerId) };
                FString const& team { playerObj->GetTeam() };
                spacelPlayerState->SetTeam(*team);
            }
        }
    }
#endif
    return initializedString;
}

void AFlyingGameMode::EndLobby()
{
    if(ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(this->GameState))
    {
        // change game state
        spacelGameState->GoToInGame();
        spacelGameState->RPCNetMulticastStartGlobalCountDown(FDateTime::UtcNow().ToUnixTimestamp(), this->RemainingGameTime);
    }

    m_timerSeconde = this->RemainingGameTime;
}

void AFlyingGameMode::EndGame()
{
    GetWorldTimerManager().ClearTimer(this->EndGameHandle);
    GetWorldTimerManager().ClearTimer(this->HandleProcessTerminationHandle);
    GetWorldTimerManager().ClearTimer(this->HandleGameSessionUpdateHandle);
    GetWorldTimerManager().ClearTimer(this->SuspendBackfillHandle);

#if WITH_GAMELIFT
    Aws::GameLift::Server::TerminateGameSession();
    Aws::GameLift::Server::ProcessEnding();
    FGenericPlatformMisc::RequestExit(false);
#endif
}

void AFlyingGameMode::LeaveLevel()
{
#if WITH_GAMELIFT
    ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(this->GameState);
    if (spacelGameState != nullptr)
    {
        spacelGameState->R_LatestEvent = "GameEnded";

        spacelGameState->R_WinningTeam = spacelGameState->GetBestTeam();

        TSharedPtr<FJsonObject> requestObj{ MakeShareable(new FJsonObject) };
        requestObj->SetStringField("winningTeam", spacelGameState->R_WinningTeam);

        auto getGameSessionIdOutcome{ Aws::GameLift::Server::GetGameSessionId() };
        if (getGameSessionIdOutcome.IsSuccess())
        {
            requestObj->SetStringField("gameSessionId", getGameSessionIdOutcome.GetResult());

            FString requestBody{};
            TSharedRef<TJsonWriter<>> writer{ TJsonWriterFactory<>::Create(&requestBody) };
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


void AFlyingGameMode::PickAWinningTeam()
{
    if(ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(this->GameState))
    {
        spacelGameState->GoToEndGame();
        spacelGameState->RPCNetMulticastStartGlobalCountDown(FDateTime::UtcNow().ToUnixTimestamp(), this->RemainingLeaveTime);

        m_timerSeconde = this->RemainingLeaveTime;
    }
}

void AFlyingGameMode::HandleProcessTermination()
{
    if (this->ProcessTerminateState.Status)
    {
        GetWorldTimerManager().ClearTimer(this->HandleProcessTerminationHandle);
        GetWorldTimerManager().ClearTimer(this->HandleGameSessionUpdateHandle);
        GetWorldTimerManager().ClearTimer(this->SuspendBackfillHandle);

#if WITH_GAMELIFT
        if(this->LatestBackfillTicketId.Len() > 0)
        {
            auto gameSessionIdOutcome { Aws::GameLift::Server::GetGameSessionId() };
            if(gameSessionIdOutcome.IsSuccess())
            {
                FString gameSessionArn { gameSessionIdOutcome.GetResult() };
                FString matchmakingConfigurationArn { this->StartGameSessionState.MatchmakingConfigurationArn };
                stopBackfillRequest(gameSessionArn, matchmakingConfigurationArn, this->LatestBackfillTicketId);
            }
        }
#endif

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
            spacelGameState->R_LatestEvent = processInterruptionMessage;
        }

        GetWorldTimerManager().SetTimer(this->EndGameHandle, this, &AFlyingGameMode::EndGame, 1.0f, false, 10.0f);
    }
}

void AFlyingGameMode::HandleGameSessionUpdate()
{
#if WITH_GAMELIFT
    if (!this->GameSessionActivated && this->StartGameSessionState.Status)
    {
        this->GameSessionActivated = true;

        m_expectedPlayers = this->StartGameSessionState.PlayerIdToPlayer;
        this->WaitingForPlayersToJoin = true;

        GetWorldTimerManager().SetTimer(this->SuspendBackfillHandle, this, &AFlyingGameMode::SuspendBackfill, 1.0f, false, (float)(this->SuspendBackfillTime));

        ASpacelGameState* spacelGameState{ Cast<ASpacelGameState>(this->GameState) };
        if (!ensure(spacelGameState != nullptr)) return;
        spacelGameState->GoToPrepare();
        m_timerSeconde = this->RemainingChooseModuleTime;
    }
    else if(this->WaitingForPlayersToJoin)
    {
        if(this->TimeSpentWaitingForPlayersToJoin < this->SuspendBackfillTime)
        {
            auto gameSessionIdOutcome = Aws::GameLift::Server::GetGameSessionId();
            if (gameSessionIdOutcome.IsSuccess())
            {
                FString gameSessionId { FString(gameSessionIdOutcome.GetResult()) };

                Aws::GameLift::Server::Model::DescribePlayerSessionsRequest describePlayerSessionsRequest {};
                describePlayerSessionsRequest.SetGameSessionId(TCHAR_TO_ANSI(*gameSessionId));
                describePlayerSessionsRequest.SetPlayerSessionStatusFilter("RESERVED");

                auto describePlayerSessionOutcome = Aws::GameLift::Server::DescribePlayerSessions(describePlayerSessionsRequest);
                if(describePlayerSessionOutcome.IsSuccess())
                {
                    auto describePlayerSessionResult = describePlayerSessionOutcome.GetResult();
                    int count { describePlayerSessionResult.GetPlayerSessionCount() };
                    if(count == 0)
                    {
                        this->UpdateGameSessionState.Reason = EUpdateReason::BACKFILL_COMPLETED;

                        this->WaitingForPlayersToJoin = false;
                        this->TimeSpentWaitingForPlayersToJoin = 0;
                        return;
                    }
                }
            }
            this->TimeSpentWaitingForPlayersToJoin++;
        }
        else
        {
            this->UpdateGameSessionState.Reason = EUpdateReason::BACKFILL_COMPLETED;
            this->WaitingForPlayersToJoin = false;
            this->TimeSpentWaitingForPlayersToJoin = 0;
        }
    }
    else if(this->UpdateGameSessionState.Reason == EUpdateReason::MATCHMAKING_DATA_UPDATED)
    {
        this->LatestBackfillTicketId = {};
        m_expectedPlayers = this->UpdateGameSessionState.PlayerIdToPlayer;

        this->WaitingForPlayersToJoin = true;
    }
    else if(this->UpdateGameSessionState.Reason == EUpdateReason::BACKFILL_CANCELLED ||
        this->UpdateGameSessionState.Reason == EUpdateReason::BACKFILL_COMPLETED ||
        this->UpdateGameSessionState.Reason == EUpdateReason::BACKFILL_FAILED ||
        this->UpdateGameSessionState.Reason == EUpdateReason::BACKFILL_TIMED_OUT)
    {
        this->LatestBackfillTicketId = {};

        UWorld const* world { this->GetWorld() };
        if (!ensure(world != nullptr)) return;

        TArray<APlayerState*> playerStates = world->GetGameState()->PlayerArray;

        TMap<FString, Aws::GameLift::Server::Model::Player> connectedPlayers;
        for(APlayerState* playerState : playerStates)
        {
            ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(playerState);
            if(spacelPlayerState != nullptr)
            {
                if(auto playerObj = m_expectedPlayers.Find(spacelPlayerState->MatchmakingPlayerId))
                {
                    connectedPlayers.Add(spacelPlayerState->MatchmakingPlayerId, *playerObj);
                }
            }
        }

        if (connectedPlayers.Num() == 0)
        {
            this->EndGame();
        }
        else if (connectedPlayers.Num() < this->MaxPlayerPerGame)
        {
            auto gameSessionIdOutcome = Aws::GameLift::Server::GetGameSessionId();
            if (gameSessionIdOutcome.IsSuccess())
            {
                FString gameSessionId = FString(gameSessionIdOutcome.GetResult());
                FString matchmakingConfigurationArn = this->StartGameSessionState.MatchmakingConfigurationArn;
                this->LatestBackfillTicketId = createBackfillRequest(gameSessionId, matchmakingConfigurationArn, connectedPlayers);
                if (this->LatestBackfillTicketId.Len() > 0)
                {
                    this->UpdateGameSessionState.Reason = EUpdateReason::BACKFILL_INITIATED;
                }
            }
        }
    }
#endif
}

void AFlyingGameMode::onRecordMatchResultResponseReceive(FHttpRequestPtr _request, FHttpResponsePtr _response, bool _bWasSuccessful)
{
    this->GetWorldTimerManager().SetTimer(this->EndGameHandle, this, &AFlyingGameMode::EndGame, 1.0f, false, 5.0f);
}

void AFlyingGameMode::SuspendBackfill()
{
    GetWorldTimerManager().ClearTimer(this->HandleGameSessionUpdateHandle);

#if WITH_GAMELIFT
    if(this->LatestBackfillTicketId.Len() > 0)
    {
        auto gameSessionIdOutcome = Aws::GameLift::Server::GetGameSessionId();
        if(gameSessionIdOutcome.IsSuccess())
        {
            FString gameSessionId { gameSessionIdOutcome.GetResult() };
            FString matchmakingConfigurationArn { this->StartGameSessionState.MatchmakingConfigurationArn };
            if(!stopBackfillRequest(gameSessionId, matchmakingConfigurationArn, this->LatestBackfillTicketId))
            {
                this->GetWorldTimerManager().SetTimer(this->SuspendBackfillHandle, this, &AFlyingGameMode::SuspendBackfill, 1.0f, false, 1.0f);
            }
        }
        else
        {
            this->GetWorldTimerManager().SetTimer(this->SuspendBackfillHandle, this, &AFlyingGameMode::SuspendBackfill, 1.0f, false, 1.0f);
        }
    }
#endif
}

FString AFlyingGameMode::createBackfillRequest(FString const& _gameSessionArn, FString const& _matchmakingConfigurationArn, TMap<FString, Aws::GameLift::Server::Model::Player> const& _players)
{
#if WITH_GAMELIFT
    Aws::GameLift::Server::Model::StartMatchBackfillRequest startMatchBackfillRequest {};
    startMatchBackfillRequest.SetGameSessionArn(TCHAR_TO_ANSI(*_gameSessionArn));
    startMatchBackfillRequest.SetMatchmakingConfigurationArn(TCHAR_TO_ANSI(*_matchmakingConfigurationArn));

    for (auto& elem : _players)
    {
        auto playerObj = elem.Value;
        startMatchBackfillRequest.AddPlayer(playerObj);
    }

    auto startMatchBackfillOutcome = Aws::GameLift::Server::StartMatchBackfill(startMatchBackfillRequest);
    if (startMatchBackfillOutcome.IsSuccess())
    {
        return startMatchBackfillOutcome.GetResult().GetTicketId();
    }
#endif //  WITH_GAMELIFT
    return {};
}

bool AFlyingGameMode::stopBackfillRequest(FString const& _gameSessionArn, FString const& _matchmakingConfigurationArn, FString const& _ticketId)
{
#if  WITH_GAMELIFT
    Aws::GameLift::Server::Model::StopMatchBackfillRequest stopMatchBackfillRequest{};
    stopMatchBackfillRequest.SetGameSessionArn(TCHAR_TO_ANSI(*_gameSessionArn));
    stopMatchBackfillRequest.SetMatchmakingConfigurationArn(TCHAR_TO_ANSI(*_matchmakingConfigurationArn));
    stopMatchBackfillRequest.SetTicketId(TCHAR_TO_ANSI(*_ticketId));

    auto stopMatchBackfillOutCome = Aws::GameLift::Server::StopMatchBackfill(stopMatchBackfillRequest);

    return stopMatchBackfillOutCome.IsSuccess();
#else
    return false;    
#endif //  WITH_GAMELIFT
}

