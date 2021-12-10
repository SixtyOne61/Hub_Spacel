// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathWidget.h"
#include "GameState/SpacelGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShipPawn.h"

void UDeathWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(world->GetGameState()))
    {
        m_currentTimer = spacelGameState->R_MinDeathTimer;
    }
}

void UDeathWidget::NativeTick(const FGeometry& _myGeometry, float _deltaSeconde)
{
    Super::NativeTick(_myGeometry, _deltaSeconde);

    updateTimer(_deltaSeconde);
}

void UDeathWidget::updateTimer(float _deltaSeconde)
{
    if (m_currentTimer > 0.0f)
    {
        m_currentTimer = FMath::Max(m_currentTimer - _deltaSeconde, 0.f);

        BP_UpdateTimer((int)FMath::RoundHalfFromZero(m_currentTimer));

        if (m_currentTimer <= 0.0f)
        {
            BP_EnableRespawn();
        }
    }
}

void UDeathWidget::Respawn()
{
    if (AShipPawn* shipPawn = this->GetOwningPlayerPawn<AShipPawn>())
    {
        shipPawn->OnAddEffectClientDelegate.AddDynamic(this, &UDeathWidget::BP_OnAddEffect);
        shipPawn->RPCServerRespawn();
    }
}