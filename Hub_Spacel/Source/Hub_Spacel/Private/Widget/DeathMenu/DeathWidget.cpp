// Fill out your copyright notice in the Description page of Project Settings.


#include "DeathWidget.h"
#include "GameState/SpacelGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/ShipPawn.h"
#include "Player/SpacelPlayerState.h"
#include "DataAsset/TeamColorDataAsset.h"
#include "DataAsset/SkillDataAsset.h"

void UDeathWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(world->GetGameState()))
    {
        m_currentTimer = spacelGameState->R_MinDeathTimer;

        WaitPlayerInfo();
    }
}

void UDeathWidget::WaitPlayerInfo()
{
    UWorld* world{ this->GetWorld() };
    if (!ensure(world != nullptr)) return;

    if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(world->GetGameState()))
    {
        // local player state
        if (AShipPawn const* shipPawn = this->GetOwningPlayerPawn<AShipPawn>())
        {
            if (ASpacelPlayerState const* localPlayerState = shipPawn->GetPlayerState<ASpacelPlayerState>())
            {
                // find killer player
                auto const& players = spacelGameState->PlayerArray;
                for (auto const* player : players)
                {
                    if (player->PlayerId == localPlayerState->R_KilledByPlayerId)
                    {
                        if (ASpacelPlayerState const* killerPlayerState = Cast<ASpacelPlayerState>(player))
                        {
                            if (this->TeamColorDataAsset != nullptr)
                            {
                                FColorsType const& _colors = this->TeamColorDataAsset->GetColorType(killerPlayerState->R_Team);
                                // setup player killer information
                                BP_Setup((ESkill)killerPlayerState->R_LowSkill,
                                    (ESkill)killerPlayerState->R_MediumSkill,
                                    (ESkill)killerPlayerState->R_HightSkill,
                                    killerPlayerState->GetPlayerName(),
                                    _colors.Logo, _colors.Color);
                                return;
                            }
                        }
                    }
                }
            }
        }
    }

    FTimerHandle handle;
    world->GetTimerManager().SetTimer(handle, this, &UDeathWidget::WaitPlayerInfo, 0.5f, false, 0.0f);
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