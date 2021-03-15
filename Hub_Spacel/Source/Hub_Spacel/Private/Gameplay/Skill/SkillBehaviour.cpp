// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Skill/SkillBehaviour.h"
#include "Player/ShipPawn.h"

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