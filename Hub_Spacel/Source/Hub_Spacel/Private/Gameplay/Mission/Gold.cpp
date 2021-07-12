// Fill out your copyright notice in the Description page of Project Settings.


#include "Gold.h"
#include "Player/ShipPawn.h"

void AGold::hit(class AShipPawn* _pawn, FString const& _team)
{
    base_type::hit(_pawn, _team);

    if (_pawn != nullptr)
    {
        _pawn->addEffect(EEffect::Gold);
        OnDestroyDelegate.broadcast(_pawn->GetUniqueID());
        this->Destroy();
    }
}