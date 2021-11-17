// Fill out your copyright notice in the Description page of Project Settings.

#include "EndScreenWidget.h"
#include "GameState/SpacelGameState.h"
#include "Player/ShipPawn.h"

void UEndScreenWidget::NativeConstruct()
{
    Super::NativeConstruct();

    setup();
}

void UEndScreenWidget::setup()
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(world->GetGameState()))
    {
        FString const& bestTeam = spacelGameState->GetBestTeam();

        if (AShipPawn* shipPawn = this->GetOwningPlayerPawn<AShipPawn>())
        {
            BP_Winner(shipPawn->Team.ToString() == bestTeam);
        }
    }
}