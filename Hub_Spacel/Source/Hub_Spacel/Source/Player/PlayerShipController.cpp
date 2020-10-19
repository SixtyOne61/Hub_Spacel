// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerShipController.h"
#include "ShipPawn.h"

void APlayerShipController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // joystick left
    this->InputComponent->BindAxis("Speed", this, &APlayerShipController::speed);
    this->InputComponent->BindAxis("FlightAttitude", this, &APlayerShipController::flightAttitude);
    // joystick right
    this->InputComponent->BindAxis("Turn", this, &APlayerShipController::turn);
    this->InputComponent->BindAxis("Up", this, &APlayerShipController::up);
}

void APlayerShipController::BeginPlay()
{
    Super::BeginPlay();

    UWorld* world = this->GetWorld();
    if (!ensure(world != nullptr)) return;

    // reduce server call, only update speed by this loop timer
    FTimerDelegate speedUpdateCallback;
    timerCallback.BindLambda([&]
    {
        if (m_lastSpeedInput.has_value())
        {
            float newPercent = FMath::Clamp(this->PercentSpeed + m_lastSpeedInput.value(), 0.0f, 100.0f);
            if (newPercent != this->PercentSpeed)
            {
                this->RPCServerSetSpeed(newPercent);
            }
            m_lastSpeedInput.reset();
        }
    });

    FTimerHandle speedTimerHandle;
    world->GetTimerManager().SetTimer(speedTimerHandle, speedUpdateCallback, 0.1f, true);
}

void APlayerShipController::speed(float _val)
{
    m_lastSpeedInput = _val;
}

void APlayerShipController::RPCServerSetSpeed_Implementation(float _val)
{
    AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn());
    if (shipPawn == nullptr)
    {
        return;
    }

    shipPawn->PercentSpeed = _val / 100.0f;
    // OnRep isn't call on server, but we need this call
    shipPawn->OnRep_PercentSpeed();
}

void APlayerShipController::flightAttitude(float _val)
{
    readInput(_val, this->PercentFlightAttitude, std::bind(&APlayerShipController::RPCServerSetFlightAttitude, this, std::placeholders::_1));
}

void APlayerShipController::RPCServerSetFlightAttitude_Implementation(float _val)
{
    AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn());
    if (shipPawn == nullptr)
    {
        return;
    }

    shipPawn->PercentFlightAttitude = _val / 100.0f;
    // OnRep isn't call on server, but we need this call
    shipPawn->OnRep_PercentFlightAttitude();
}

void APlayerShipController::turn(float _val)
{
    readInput(_val, this->Turn, std::bind(&APlayerShipController::RPCServerSetTurn, this, std::placeholders::_1));
}

void APlayerShipController::RPCServerSetTurn_Implementation(float _val)
{
    AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn());
    if (shipPawn == nullptr)
    {
        return;
    }

    shipPawn->PercentTurn = _val / 100.0f;
    // OnRep isn't call on server, but we need this call
    shipPawn->OnRep_PercentTurn();
}

void APlayerShipController::up(float _val)
{
    readInput(_val, this->Up, std::bind(&APlayerShipController::RPCServerSetUp, this, std::placeholders::_1));
}

void APlayerShipController::RPCServerSetUp_Implementation(float _val)
{
    AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn());
    if (shipPawn == nullptr)
    {
        return;
    }

    shipPawn->PercentUp = _val / 100.0f;
    // OnRep isn't call on server, but we need this call
    shipPawn->OnRep_PercentUp();
}

void APlayerShipController::readInput(int const& _val, float& _in, std::function<void(float)> _fnc)
{
    std::optional<float> newPercent { };
    if (FMath::IsNearlyZero(_val, 0.05f))
    {
        if (_in != 0.0f)
        {
            newPercent = 0.0f;
        }
    }
    else if (_val > 0.0f)
    {
        if (_in <= 0.0f)
        {
            newPercent = 0.0f;
        }
        newPercent = FMath::Clamp(_in + _val, 0.0f, 100.0f);
    }
    else if (_val < 0.0f)
    {
        if (_in >= 0.0f)
        {
            newPercent = 0.0f;
        }
        newPercent = FMath::Clamp(_in + _val, -100.0f, 0.0f);
    }

    if (newPercent.has_value() && newPercent.value() != _in)
    {
        _fnc(_in);
    }
}