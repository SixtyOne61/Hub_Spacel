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

void APlayerShipController::speed(float _val)
{
    if (FMath::IsNearlyZero(_val, 0.05f))
    {
        return;
    }

    this->RPCServerSetSpeed(_val);
}

void APlayerShipController::RPCServerSetSpeed_Implementation(float _val)
{
    // TO DO better
    float newPercent = FMath::Clamp(this->PercentSpeed + _val, 0.0f, 100.0f);
    if (newPercent != this->PercentSpeed)
    {
        this->PercentSpeed = newPercent;
        AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn());
        if (shipPawn == nullptr)
        {
            return;
        }

        shipPawn->PercentSpeed = this->PercentSpeed / 100.0f;
    }
}

void APlayerShipController::flightAttitude(float _val)
{
    readInput(_val, this->FlightAttitude, std::bind(&APlayerShipController::RPCServerSetFlightAttitude, this, std::placeholders::_1));
}

void APlayerShipController::RPCServerSetFlightAttitude_Implementation(float _val)
{
    AShipPawn* shipPawn = Cast<AShipPawn>(this->GetPawn());
    if (shipPawn == nullptr)
    {
        return;
    }

    shipPawn->PercentFlightAttitude = _val;
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

    shipPawn->PercentTurn = _val;
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

    shipPawn->PercentUp = _val;
}

void APlayerShipController::readInput(int const& _val, float& _in, std::function<void(float)> _fnc)
{
    if (FMath::IsNearlyZero(_val, 0.05f))
    {
        if (_in != 0.0f)
        {
            _in = 0.0f;
            _fnc(_in);
        }
    }
    else if (_val > 0.0f)
    {
        if (_in <= 0.0f)
        {
            _in = 1.0f;
            _fnc(_in);
        }
    }
    else if (_val < 0.0f)
    {
        if (_in >= 0.0f)
        {
            _in = -1.0f;
            _fnc(_in);
        }
    }
}