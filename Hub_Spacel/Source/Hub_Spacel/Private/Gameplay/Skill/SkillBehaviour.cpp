// Fill out your copyright notice in the Description page of Project Settings.


#include "Gameplay/Skill/SkillBehaviour.h"
#include "Player/Common/CommonPawn.h"
#include "Player/ShipPawn.h"
#include "Player/SpacelPlayerState.h"
#include "Player/MetricComponent.h"
#include "GameState/SpacelGameState.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "DataAsset/UniqueSkillDataAsset.h"

SkillBehaviour::SkillBehaviour(ACommonPawn* _pawn, ENetMode _netMode, class UUniqueSkillDataAsset const* _data)
    : m_pawn(_pawn)
    , m_data(_data)
    , m_netMode(_netMode)
{
}

ESkillReturn SkillHealPack::onStart()
{
    if (get() == nullptr) return ESkillReturn::InternError;
    if (m_data == nullptr) return ESkillReturn::InternError;

    if (AShipPawn* pawn = Cast<AShipPawn>(get()))
    {
        int healPackMatiere = m_data->MatiereNeeded;
        if (pawn->getMatiere() >= healPackMatiere)
        {
            pawn->addMatiere(healPackMatiere * -1, EMatiereOrigin::Lost);
            get()->addEffect(EEffect::HealZone);
            m_timer = 0.0f;
            return ESkillReturn::Success;
        }
        else
        {
            return ESkillReturn::NoMater;
        }
    }
    else
    {
        return ESkillReturn::InternError;
    }
}

void SkillHealPack::onEnd()
{
    if (get() == nullptr) return;
    get()->removeEffect(EEffect::HealZone);
}

void SkillHealPack::onEndCountDown()
{

}

void SkillHealPack::useTick(float _deltaSeconde)
{
    m_timer += _deltaSeconde;
    if (m_timer >= 0.8f)
    {
        searchPlayerAround();
        m_timer = 0.0f;
    }
}

void SkillHealPack::searchPlayerAround() const
{
    if (get() == nullptr) return;
    if (m_data == nullptr) return;

    FName const& team = get()->Team;
    FVector const& location = get()->GetActorLocation();

    if (auto gameState = UGameplayStatics::GetGameState(get()->GetWorld()))
    {
        int totalHeal { 0 };
        auto playerStates = gameState->PlayerArray;
        for (auto playerState : playerStates)
        {
            if (auto spacelPlayerState = Cast<ASpacelPlayerState>(playerState))
            {
                if (*spacelPlayerState->R_Team == team)
                {
                    if (auto pawn = Cast<AShipPawn>(spacelPlayerState->GetPawn()))
                    {
                        // not us
                        if (get()->GetUniqueID() != pawn->GetUniqueID())
                        {
                            FVector const& allyLocation = pawn->GetActorLocation();
                            if (FVector::Distance(location, allyLocation) <= 2000.0f)
                            {
                                pawn->heal(m_data->Value);
                                totalHeal += m_data->Value;
                            }
                        }
                    }
                }
            }
        }

        if (totalHeal != 0)
        {
            // update metric on our player
            if (UMetricComponent* metricComponent = Cast<UMetricComponent>(get()->GetComponentByClass(UMetricComponent::StaticClass())))
            {
                metricComponent->updateMetric<SMetricAdd, int>(EMetric::Heal, { m_data->Value });
            }
        }
    }
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

ESkillReturn SkillBulletStun::onStart()
{
    if (get<AShipPawn>() == nullptr) return ESkillReturn::InternError;

    get()->addEffect(EEffect::BulletStun);
    return ESkillReturn::Success;
}

void SkillBulletStun::onEnd()
{
    if (get() == nullptr) return;
    get()->removeEffect(EEffect::BulletStun);
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

ESkillReturn SkillShotgun::onStart()
{
    if (get<AShipPawn>() == nullptr) return ESkillReturn::InternError;

    get<AShipPawn>()->fireShotgun();
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

TUniquePtr<SkillBehaviour> SkillFactory::create(ESkill _skill, class ACommonPawn* _pawn, class UUniqueSkillDataAsset const* _data, ENetMode _netMode)
{
    switch (_skill)
    {
        case ESkill::EscapeMode : return MakeUnique<SkillEscapeMode>(_pawn, _netMode, _data);
        case ESkill::Missile : return MakeUnique<SkillMissile>(_pawn, _netMode, _data);
        case ESkill::Farmer: return MakeUnique<SkillFarmer>(_pawn, _netMode, _data);
        case ESkill::BulletStun: return MakeUnique<SkillBulletStun>(_pawn, _netMode, _data);
        case ESkill::MetaFormAttack: return MakeUnique<SkillMetaFormAttack>(_pawn, _netMode, _data);
        case ESkill::MetaFormProtection: return MakeUnique<SkillMetaFormProtection>(_pawn, _netMode, _data);
        case ESkill::MetaFormSupport: return MakeUnique<SkillMetaFormSupport>(_pawn, _netMode, _data);
        case ESkill::Shotgun: return MakeUnique<SkillShotgun>(_pawn, _netMode, _data);
        case ESkill::HealPack: return MakeUnique<SkillHealPack>(_pawn, _netMode, _data);
        case ESkill::Emergency: return MakeUnique<SkillEmergency>(_pawn, _netMode, _data);
        case ESkill::Repair: return MakeUnique<SkillRepair>(_pawn, _netMode, _data);
        default: ensure(false); break;
    }

    return nullptr;
}