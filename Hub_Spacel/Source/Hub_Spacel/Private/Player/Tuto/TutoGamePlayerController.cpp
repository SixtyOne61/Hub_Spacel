// Fill out your copyright notice in the Description page of Project Settings.


#include "TutoGamePlayerController.h"
#include "Player/Tuto/TutoPawn.h"

void ATutoGamePlayerController::Tick(float _deltaTime)
{
    Super::Tick(_deltaTime);

    FVector hitLoc{ FVector::ZeroVector };
    FVector mouseWorldLocation{}, mouseWorldDirection{};
    TWeakObjectPtr<AActor> hit{};
    bool hasDeproj{}, hasHit{};
    hitResultUnderCursor(hasDeproj, hasHit, hitLoc, mouseWorldLocation, mouseWorldDirection, hit);

    if (hasDeproj)
    {
        updateMouseLocation<ATutoPawn>(mouseWorldLocation, mouseWorldDirection, hitLoc);
    }

    updatePawnData<ATutoPawn>(_deltaTime);
}

void ATutoGamePlayerController::forward(float _value)
{
    m_data.m_lastForwardInput = _value;
}

void ATutoGamePlayerController::horizontalStraf(float _value)
{
    m_data.m_lastHorizontalStrafInput = _value;
}

void ATutoGamePlayerController::verticalStraf(float _value)
{
    m_data.m_lastVerticalStrafInput = _value;
}

void ATutoGamePlayerController::flightAttitude(float _value)
{
    m_data.m_lastFlightAttitudeInput = _value;
}

void ATutoGamePlayerController::fireOn()
{

}

void ATutoGamePlayerController::fireOff()
{

}

void ATutoGamePlayerController::lock()
{

}
