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
}

void ATutoGamePlayerController::forward(float _value)
{
    ensure(false);
}

void ATutoGamePlayerController::horizontalStraf(float _value)
{

}

void ATutoGamePlayerController::verticalStraf(float _value)
{

}

void ATutoGamePlayerController::flightAttitude(float _value)
{

}

void ATutoGamePlayerController::fireOn()
{

}

void ATutoGamePlayerController::fireOff()
{

}

void ATutoGamePlayerController::returnToMainMenu()
{

}

void ATutoGamePlayerController::lock()
{

}

void ATutoGamePlayerController::skill(float _slot)
{

}
