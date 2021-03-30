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
    m_pawn->addEffect(EEffect::EscapeMode);
    return true;
}

void SkillEscapeMode::onEnd()
{
    if (m_pawn == nullptr) return;
    m_pawn->removeEffect(EEffect::EscapeMode);
}

void SkillEscapeMode::onEndCountDown()
{
}

bool SkillSpecialAttack::onStart()
{
    if (m_pawn == nullptr || !m_pawn->hasEffect(EEffect::TargetLock)) return false;

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
        pawn->addEffect(EEffect::Shield);
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
        pawn->removeEffect(EEffect::Shield);
    }
}

bool SkillSpecialSupport::onStart()
{
    if (m_pawn == nullptr || !m_pawn->hasEffect(EEffect::TargetLock)) return false;

    m_pawn->emp();
    return true;
}

bool SkillMetaFormAttack::onStart()
{
    if (m_pawn == nullptr) return false;

    m_pawn->addEffect(EEffect::MetaFormAttack);
    return true;
}

void SkillMetaFormAttack::onEnd()
{
    if (m_pawn != nullptr)
    {
        m_pawn->removeEffect(EEffect::MetaFormAttack);
    }
}

bool SkillMetaFormProtection::onStart()
{
    if (m_pawn == nullptr) return false;

    m_pawn->addEffect(EEffect::MetaFormProtection);
    return true;
}

void SkillMetaFormProtection::onEnd()
{
    if (m_pawn != nullptr)
    {
        m_pawn->removeEffect(EEffect::MetaFormProtection);
    }
}

bool SkillMetaFormSupport::onStart()
{
    if (m_pawn == nullptr) return false;

    m_pawn->addEffect(EEffect::MetaFormSupport);
    return true;
}

void SkillMetaFormSupport::onEnd()
{
    if (m_pawn != nullptr)
    {
        m_pawn->removeEffect(EEffect::MetaFormSupport);
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
        case ESkill::MetaFormAttack: return MakeUnique<SkillMetaFormAttack>(_pawn, _netMode);
        case ESkill::MetaFormProtection: return MakeUnique<SkillMetaFormProtection>(_pawn, _netMode);
        case ESkill::MetaFormSupport: return MakeUnique<SkillMetaFormSupport>(_pawn, _netMode);
    }

    return nullptr;
}