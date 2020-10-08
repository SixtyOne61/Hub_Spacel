// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerShipController.h"
#include "ShipPawn.h"

void APlayerShipController::SetupInputComponent()
{
    Super::SetupInputComponent();

    this->InputComponent->BindAxis("Speed", this, &APlayerShipController::speed);
    this->InputComponent->BindAxis("Turn", this, &APlayerShipController::turn);
    this->InputComponent->BindAxis("FlightAttitude", this, &APlayerShipController::flightAttitude);
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

void APlayerShipController::turn(float _val)
{
    if (FMath::IsNearlyZero(_val, 0.05f))
    {
        return;
    }

    this->RPCServerSetTurn(_val);
}

void APlayerShipController::RPCServerSetTurn_Implementation(float _val)
{

}

void APlayerShipController::flightAttitude(float _val)
{
    if (FMath::IsNearlyZero(_val, 0.05f))
    {
        return;
    }

    this->RPCServerSetFlightAttitude(_val);
}


void APlayerShipController::RPCServerSetFlightAttitude_Implementation(float _val)
{

}

void APlayerShipController::up(float _val)
{
    if (FMath::IsNearlyZero(_val, 0.05f))
    {
        return;
    }

    this->RPCServerSetUp(_val);
}

void APlayerShipController::RPCServerSetTurn_Implementation(float _val)
{

}