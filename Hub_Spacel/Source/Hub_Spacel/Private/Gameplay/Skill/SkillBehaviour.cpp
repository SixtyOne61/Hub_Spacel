// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Skill/SkillBehaviour.h"
#include "Player/Common/CommonPawn.h"
#include "Player/ShipPawn.h"
#include "GameState/SpacelGameState.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerState.h"
#include "Player/SpacelPlayerState.h"

SkillBehaviour::SkillBehaviour(ACommonPawn* _pawn, ENetMode _netMode)
    : m_pawn(_pawn)
    , m_netMode(_netMode)
{
}

ESkillReturn SkillHealPack::onStart()
{
    if (get<AShipPawn>() == nullptr) return ESkillReturn::InternError;
    return get<AShipPawn>()->spawnHealPack();
}

ESkillReturn SkillRepair::onStart()
{
    if (get<AShipPawn>() == nullptr) return ESkillReturn::InternError;
    return get<AShipPawn>()->onRepair();
}

ESkillReturn SkillEscapeMode::onStart()
{
    if (get() == nullptr) return ESkillReturn::InternError;
    get()->addEffect(EEffect::EscapeMode);
    return ESkillReturn::Success;
}

void SkillEscapeMode::onEnd()
{
    if (get() == nullptr) return;
    get()->removeEffect(EEffect::EscapeMode);
}

void SkillEscapeMode::onEndCountDown()
{
}

ESkillReturn SkillMissile::onStart()
{
    if (get<AShipPawn>() == nullptr) return ESkillReturn::InternError;

    get<AShipPawn>()->addEffect(EEffect::Missile);
    return ESkillReturn::Success;
}

void SkillMissile::onEnd()
{
    if (get() == nullptr) return;
    get()->removeEffect(EEffect::Missile);
}

ESkillReturn SkillFarmer::onStart()
{
    if (get() == nullptr) return ESkillReturn::InternError;

    get()->addEffect(EEffect::Farmer);
    return ESkillReturn::Success;
}

void SkillFarmer::onEnd()
{
    if (get() == nullptr) return;
    get()->removeEffect(EEffect::Farmer);
}

ESkillReturn SkillSpecialSupport::onStart()
{
    if (get<AShipPawn>() == nullptr) return ESkillReturn::InternError;
    return get<AShipPawn>()->spawnEmp();
}

ESkillReturn SkillMetaFormAttack::onStart()
{
    if (get() == nullptr) return ESkillReturn::InternError;

    get()->addEffect(EEffect::MetaFormAttack);
    return ESkillReturn::Success;
}

void SkillMetaFormAttack::onEnd()
{
    if (get() != nullptr)
    {
        get()->removeEffect(EEffect::MetaFormAttack);
    }
}

ESkillReturn SkillMetaFormProtection::onStart()
{
    if (get() == nullptr) return ESkillReturn::InternError;

    get()->addEffect(EEffect::MetaFormProtection);
    return ESkillReturn::Success;
}

void SkillMetaFormProtection::onEnd()
{
    if (get() != nullptr)
    {
        get()->removeEffect(EEffect::MetaFormProtection);
    }
}

ESkillReturn SkillMetaFormSupport::onStart()
{
    if (get() == nullptr) return ESkillReturn::InternError;

    get()->addEffect(EEffect::MetaFormSupport);
    return ESkillReturn::Success;
}

void SkillMetaFormSupport::onEnd()
{
    if (get() != nullptr)
    {
        get()->removeEffect(EEffect::MetaFormSupport);
    }
}

ESkillReturn SkillKatyusha::onStart()
{
    if (get<AShipPawn>() == nullptr) return ESkillReturn::InternError;

    get<AShipPawn>()->spawnKatyusha();
    return ESkillReturn::Success;
}

ESkillReturn SkillEmergency::onStart()
{
    if (get<AShipPawn>() == nullptr) return ESkillReturn::InternError;
    auto ret = get<AShipPawn>()->onSwapEmergency();

    if (ret == ESkillReturn::Success)
    {
        onEndCountDown();
    }
    return ret;
}

void SkillEmergency::onEndCountDown()
{
    if (get<AShipPawn>() == nullptr) return;
    get<AShipPawn>()->onEmergencyCountDownEnd();
}

TUniquePtr<SkillBehaviour> SkillFactory::create(ESkill _skill, class ACommonPawn* _pawn, ENetMode _netMode)
{
    switch (_skill)
    {
        case ESkill::EscapeMode : return MakeUnique<SkillEscapeMode>(_pawn, _netMode);
        case ESkill::Missile : return MakeUnique<SkillMissile>(_pawn, _netMode);
        case ESkill::Farmer: return MakeUnique<SkillFarmer>(_pawn, _netMode);
        case ESkill::Emp: return MakeUnique<SkillSpecialSupport>(_pawn, _netMode);
        case ESkill::MetaFormAttack: return MakeUnique<SkillMetaFormAttack>(_pawn, _netMode);
        case ESkill::MetaFormProtection: return MakeUnique<SkillMetaFormProtection>(_pawn, _netMode);
        case ESkill::MetaFormSupport: return MakeUnique<SkillMetaFormSupport>(_pawn, _netMode);
        case ESkill::Katyusha: return MakeUnique<SkillKatyusha>(_pawn, _netMode);
        case ESkill::HealPack: return MakeUnique<SkillHealPack>(_pawn, _netMode);
        case ESkill::Emergency: return MakeUnique<SkillEmergency>(_pawn, _netMode);
        case ESkill::Repair: return MakeUnique<SkillRepair>(_pawn, _netMode);
        default: ensure(false); break;
    }

    return nullptr;
}