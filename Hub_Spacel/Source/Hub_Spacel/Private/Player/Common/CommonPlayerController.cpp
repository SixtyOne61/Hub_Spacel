// Fill out your copyright notice in the Description page of Project Settings.


#include "CommonPlayerController.h"

void ACommonPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    this->InputComponent->BindAxis("Forward", this, &ACommonPlayerController::forward);
    this->InputComponent->BindAxis("HorizontalStraf", this, &ACommonPlayerController::horizontalStraf);
    this->InputComponent->BindAxis("VerticalStraf", this, &ACommonPlayerController::verticalStraf);
    this->InputComponent->BindAxis("FlightAttitude", this, &ACommonPlayerController::flightAttitude);
    this->InputComponent->BindAxis("Skill", this, &ACommonPlayerController::skill);
    
    this->InputComponent->BindAction("Fire", IE_Pressed, this, &ACommonPlayerController::fireOn);
    this->InputComponent->BindAction("Fire", IE_Released, this, &ACommonPlayerController::fireOff);
    this->InputComponent->BindAction("ReturnToMainMenu", IE_Pressed, this, &ACommonPlayerController::returnToMainMenu);
    this->InputComponent->BindAction("Lock", IE_Pressed, this, &ACommonPlayerController::lock);
}

void ACommonPlayerController::Tick(float _deltaTime)
{
    Super::Tick(_deltaTime);
}

void ACommonPlayerController::hitResultUnderCursor(bool& _hasDeproj, bool& _hasHit, FVector& _out, FVector& _mouseWorldLocation, FVector& _mouseWorldDirection, TWeakObjectPtr<AActor>& _hit)
{
    if (this->DeprojectMousePositionToWorld(_mouseWorldLocation, _mouseWorldDirection))
    {
        _hasDeproj = true;

        FHitResult hit;
        if (getHitResultUnderCursor(ECollisionChannel::ECC_MAX, false, hit, this->GetPawn()))
        {
            _out = hit.Location;
            _hit = hit.Actor;
            _hasHit = true;
            return;
        }
    }
    _hasHit = false;
}

bool ACommonPlayerController::getHitResultUnderCursor(ECollisionChannel TraceChannel, bool bTraceComplex, FHitResult& HitResult, AActor* _ignoreActor)
{
    ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(Player);
    bool bHit = false;
    if (LocalPlayer && LocalPlayer->ViewportClient)
    {
        FVector2D MousePosition;
        if (LocalPlayer->ViewportClient->GetMousePosition(MousePosition))
        {
            FCollisionQueryParams CollisionQueryParams(SCENE_QUERY_STAT(ClickableTrace), bTraceComplex);
            CollisionQueryParams.AddIgnoredActor(_ignoreActor);
            bHit = GetHitResultAtScreenPosition(MousePosition, TraceChannel, CollisionQueryParams, HitResult);
        }
    }

    if (!bHit)	//If there was no hit we reset the results. This is redundant but helps Blueprint users
    {
        HitResult = FHitResult();
    }

    return bHit;
}