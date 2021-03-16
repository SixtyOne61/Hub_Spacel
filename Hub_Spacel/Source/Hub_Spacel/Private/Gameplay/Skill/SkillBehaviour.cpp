// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Skill/SkillBehaviour.h"
#include "Player/ShipPawn.h"
#include "GameState/SpacelGameState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerState.h"

SkillBehaviour::SkillBehaviour(AShipPawn* _pawn, ENetMode _netMode)
    : m_pawn(_pawn)
    , m_netMode(_netMode)
{
}

bool SkillRepairProtection::onStart()
{
    if(m_pawn == nullptr) return false;
    m_pawn->OnRepairProtectionDelegate.Broadcast();
    return true;
}

bool SkillRepairSupport::onStart()
{
    if (m_pawn == nullptr) return false;
    m_pawn->OnRepairSupportDelegate.Broadcast();
    return true;
}

bool SkillEscapeMode::onStart()
{
    if (m_pawn == nullptr) return false;
    m_pawn->setIsEscape(true);
    return true;
}

void SkillEscapeMode::onEnd()
{
    if (m_pawn == nullptr) return;
    m_pawn->setIsEscape(false);
}

void SkillEscapeMode::onEndCountDown()
{
}

bool SkillSpecialAttack::onStart()
{
    if (m_pawn == nullptr || !m_pawn->isTargetPlayer()) return false;

    m_pawn->launchMissile();
    return true;
}

void SkillSpecialProtection::fillPlayer(FName const& _team, TArray<AShipPawn*>& _pawns) const
{
    _pawns.Empty();
    if (AGameStateBase* gameState = UGameplayStatics::GetGameState(m_pawn->GetWorld()))
    {
        for (APlayerState const* playerState : gameState->PlayerArray)
        {
            if (playerState)
            {
                if (AShipPawn* pawn = Cast<AShipPawn>(playerState->GetPawn()))
                {
                    if (pawn->Team == _team)
                    {
                        _pawns.Add(pawn);
                    }
                }
            }
        }
    }
}

bool SkillSpecialProtection::onStart()
{
    if (m_pawn == nullptr) return false;

    FName team = m_pawn->Team;
    TArray<AShipPawn*> pawns;
    fillPlayer(team, pawns);

    for (AShipPawn* pawn : pawns)
    {
        pawn->addShield();
    }
    return true;
}

void SkillSpecialProtection::onEnd()
{
    if (m_pawn == nullptr) return;

    FName team = m_pawn->Team;
    TArray<AShipPawn*> pawns;
    fillPlayer(team, pawns);

    for (AShipPawn* pawn : pawns)
    {
        pawn->removeShield();
    }
}

TUniquePtr<SkillBehaviour> SkillFactory::create(ESkill _skill, class AShipPawn* _pawn, ENetMode _netMode)
{
    switch (_skill)
    {
        case ESkill::RepairProtection : return MakeUnique<SkillRepairProtection>(_pawn, _netMode);
        case ESkill::RepairSupport : return MakeUnique<SkillRepairSupport>(_pawn, _netMode);
        case ESkill::GiveAlly1 : return MakeUnique<SkillGiveAlly1>(_pawn, _netMode);
        case ESkill::GiveAlly2 : return MakeUnique<SkillGiveAlly2>(_pawn, _netMode);
        case ESkill::EscapeMode : return MakeUnique<SkillEscapeMode>(_pawn, _netMode);
        case ESkill::SpecialAttack : return MakeUnique<SkillSpecialAttack>(_pawn, _netMode);
        case ESkill::SpecialProtection: return MakeUnique<SkillSpecialProtection>(_pawn, _netMode);
        case ESkill::SpecialSupport: return MakeUnique<SkillSpecialSupport>(_pawn, _netMode);
    }

    return nullptr;
}