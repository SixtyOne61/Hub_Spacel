// Fill out your copyright notice in the Description page of Project Settings.


#include "TutoGamePlayerController.h"

void ATutoGamePlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    this->InputComponent->BindAxis("Forward", this, &ATutoGamePlayerController::forward);
    this->InputComponent->BindAxis("HorizontalStraf", this, &ATutoGamePlayerController::horizontalStraf);
    this->InputComponent->BindAxis("VerticalStraf", this, &ATutoGamePlayerController::verticalStraf);
    this->InputComponent->BindAxis("FlightAttitude", this, &ATutoGamePlayerController::flightAttitude);
    this->InputComponent->BindAxis("Skill", this, &ATutoGamePlayerController::skill);

    this->InputComponent->BindAction("Fire", IE_Pressed, this, &ATutoGamePlayerController::fireOn);
    this->InputComponent->BindAction("Fire", IE_Released, this, &ATutoGamePlayerController::fireOff);
    this->InputComponent->BindAction("ReturnToMainMenu", IE_Pressed, this, &ATutoGamePlayerController::returnToMainMenu);
    this->InputComponent->BindAction("Lock", IE_Pressed, this, &ATutoGamePlayerController::lock);
}

void ATutoGamePlayerController::forward(float _value)
{

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
