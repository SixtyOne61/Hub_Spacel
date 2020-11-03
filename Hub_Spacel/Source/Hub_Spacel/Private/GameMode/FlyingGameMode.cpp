// Fill out your copyright notice in the Description page of Project Settings.


#include "FlyingGameMode.h"

AFlyingGameMode::AFlyingGameMode()
{
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

            auto processEndingOutcome = Aws::GameLift::Server::ProcessEnding();

            if(processEndingOutcome.IsSuccess())
            {
                state->Status = true;
                FGenericPlatformMisc::RequestExit(false);
            }
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
            FString key, value = { };

            if(str.Split("=", &key, &value))
            {
                if(key.Equals("port"))
                {
                    port = FCString::Atoi(*value);
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
}