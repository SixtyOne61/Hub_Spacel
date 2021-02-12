// Fill out your copyright notice in the Description page of Project Settings.


#include "GamePlayerController.h"
#include "Player/ShipPawn.h"
#include "Player/SpacelPlayerState.h"
#include "DataAsset/PlayerDataAsset.h"
#include "DataAsset/InputDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "GameState/SpacelGameState.h"
#include "GameMode/FlyingGameMode.h"

void AGamePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    this->InputComponent->BindAxis("Forward", this, &AGamePlayerController::forward);
    this->InputComponent->BindAxis("HorizontalStraf", this, &AGamePlayerController::horizontalStraf);
    this->InputComponent->BindAxis("VerticalStraf", this, &AGamePlayerController::verticalStraf);
    this->InputComponent->BindAxis("FlightAttitude", this, &AGamePlayerController::flightAttitude);
}

void AGamePlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (this->IsLocalController())
    {
        if (AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn()))
        {
            ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld()));
            if (spacelGameState != nullptr)
            {
                spacelGameState->OnStartGameDelegate.AddDynamic(this, &AGamePlayerController::StartGame);
            }
        }
    }
}

void AGamePlayerController::Tick(float _deltaTime)
{
    Super::Tick(_deltaTime);

    AShipPawn* shipPawn { Cast<AShipPawn>(this->GetPawn()) };
    if (this->IsLocalController())
    {
        FVector mouseWorldLocation{}, mouseWorldDirection{};
        if (this->DeprojectMousePositionToWorld(mouseWorldLocation, mouseWorldDirection))
        {
            this->RPCServerUpdateMouseLocation(mouseWorldLocation, mouseWorldDirection);
        }
    }
    else if (this->GetNetMode() == ENetMode::NM_DedicatedServer && shipPawn != nullptr)
    {
        shipPawn->RU_PercentSpeed = m_data.m_forward.addValue(m_data.m_lastForwardInput, _deltaTime);
        shipPawn->PercentHorizontalStraf = m_data.m_horizontalStraf.addValue(m_data.m_lastHorizontalStrafInput, _deltaTime);
        shipPawn->PercentVerticalStraf = m_data.m_verticalStraf.addValue(m_data.m_lastVerticalStrafInput, _deltaTime);
        shipPawn->PercentFlightAttitude = m_data.m_flightAttitude.addValue(m_data.m_lastFlightAttitudeInput, _deltaTime);
    }
}

void AGamePlayerController::RPCServerUpdateMouseLocation_Implementation(FVector const& _loc, FVector const& _dir)
{
    AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn());
    if (m_enableInput && shipPawn != nullptr)
    {
        shipPawn->lookAt(_loc, _dir);
    }
}

void AGamePlayerController::RPCServerForward_Implementation(float _val)
{
    if (m_enableInput)
    {
        m_data.m_lastForwardInput = _val;
    }
}

void AGamePlayerController::RPCServerHorizontalStraf_Implementation(float _val)
{
    if (m_enableInput)
    {
        m_data.m_lastHorizontalStrafInput = _val;
    }
}

void AGamePlayerController::RPCServerVerticalStraf_Implementation(float _val)
{
    if (m_enableInput)
    {
        m_data.m_lastVerticalStrafInput = _val;
    }
}

void AGamePlayerController::RPCServerFlightAttitude_Implementation(float _val)
{
    if (m_enableInput)
    {
        m_data.m_lastFlightAttitudeInput = _val;
    }
}

void AGamePlayerController::forward(float _value)
{
    this->RPCServerForward(_value);
}

void AGamePlayerController::horizontalStraf(float _value)
{
    this->RPCServerHorizontalStraf(_value);
}

void AGamePlayerController::verticalStraf(float _value)
{
    this->RPCServerVerticalStraf(_value);
}

void AGamePlayerController::flightAttitude(float _value)
{
    this->RPCServerFlightAttitude(_value);
}

void AGamePlayerController::StartGame()
{
    this->RPCServerStartGame();
}

void AGamePlayerController::RPCServerStartGame_Implementation()
{
    AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn());
    if (shipPawn == nullptr || shipPawn->PlayerDataAsset == nullptr)
    {
        return;
    }

    auto lb_init = [](UInputDataAsset* _data, DelayValue& _value)
    {
        if (_data != nullptr)
        {
            _value.init(_data->TimeUp, _data->TimeDown, _data->TimeReset);
        }
    };

    lb_init(shipPawn->PlayerDataAsset->ForwardInput, m_data.m_forward);
    lb_init(shipPawn->PlayerDataAsset->HorizontalStrafInput, m_data.m_horizontalStraf);
    lb_init(shipPawn->PlayerDataAsset->VerticalStrafInput, m_data.m_verticalStraf);
    lb_init(shipPawn->PlayerDataAsset->FlightAttitudeInput, m_data.m_flightAttitude);

    m_enableInput = true;
}

void AGamePlayerController::Restart()
{
    AFlyingGameMode* flyingGameMode = Cast<AFlyingGameMode>(UGameplayStatics::GetGameMode(this->GetWorld()));
    if (flyingGameMode == nullptr)
    {
        return;
    }

    ASpacelPlayerState* spacelPlayerState = this->GetPlayerState<ASpacelPlayerState>();
    if (spacelPlayerState)
    {
        flyingGameMode->Restart(this, spacelPlayerState->PlayerStartTransform);
    }
}