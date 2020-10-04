// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerShipController.h"
#include "ShipPawn.h"

void APlayerShipController::SetupInputComponent()
{
    Super::SetupInputComponent();

    this->InputComponent->BindAxis("Speed", this, &APlayerShipController::speed);
}

void APlayerShipController::speed(float _val)
{
    if(FMath::IsNearlyZero(_val, 0.05f))
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