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

bool SkillRepairProtection::onStart()
{
    if(get<AShipPawn>()) return false;
    get<AShipPawn>()->OnRepairProtectionDelegate.Broadcast();
    return true;
}

bool SkillRepairSupport::onStart()
{
    if (get<AShipPawn>() == nullptr) return false;
    get<AShipPawn>()->OnRepairSupportDelegate.Broadcast();
    return true;
}

bool giveMatiere(AShipPawn* _shipPawn, uint8 _id)
{
    if (ASpacelPlayerState* localSpacelPlayerState = _shipPawn->GetPlayerState<ASpacelPlayerState>())
    {
        FString const& localTeam = localSpacelPlayerState->R_Team;
        TArray<APlayerState*> const& playerStates = _shipPawn->GetWorld()->GetGameState()->PlayerArray;
        uint8 i = 0;
        for (APlayerState const* playerState : playerStates)
        {
            if (ASpacelPlayerState const* spacelPlayerState = Cast<ASpacelPlayerState>(playerState))
            {
                if (spacelPlayerState->R_Team == localTeam)
                {
                    if (i == _id)
                    {
                        
                    }
                    ++i;
                }
            }
        }
    }
    return false;
}

bool SkillGiveAlly1::onStart()
{
    if (get<AShipPawn>() == nullptr) return false;
    get<AShipPawn>()->giveMatiereToAlly(0);
    return true;
}

bool SkillGiveAlly2::onStart()
{
    if (get<AShipPawn>() == nullptr) return false;
    get<AShipPawn>()->giveMatiereToAlly(1);
    return true;
}

bool SkillEscapeMode::onStart()
{
    if (get() == nullptr) return false;
    get()->addEffect(EEffect::EscapeMode);
    return true;
}

void SkillEscapeMode::onEnd()
{
    if (get() == nullptr) return;
    get()->removeEffect(EEffect::EscapeMode);
}

void SkillEscapeMode::onEndCountDown()
{
}

bool SkillSpecialAttack::onStart()
{
    if (get<AShipPawn>() == nullptr || !get<AShipPawn>()->hasEffect(EEffect::TargetLock)) return false;

    get<AShipPawn>()->launchMissile();
    return true;
}

void SkillSpecialProtection::fillPlayer(FName const& _team, TArray<AShipPawn*>& _pawns) const
{
    if(get() == nullptr) return;

    _pawns.Empty();
    if (AGameStateBase* gameState = UGameplayStatics::GetGameState(get()->GetWorld()))
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
    if (get() == nullptr) return false;

    FName team = get()->Team;
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
    if (get() == nullptr) return;

    FName team = get()->Team;
    TArray<AShipPawn*> pawns;
    fillPlayer(team, pawns);

    for (AShipPawn* pawn : pawns)
    {
        pawn->removeEffect(EEffect::Shield);
    }
}

bool SkillSpecialSupport::onStart()
{
    if (get<AShipPawn>() == nullptr || !get<AShipPawn>()->hasEffect(EEffect::TargetLock)) return false;

    get<AShipPawn>()->emp();
    return true;
}

bool SkillMetaFormAttack::onStart()
{
    if (get() == nullptr) return false;

    get()->addEffect(EEffect::MetaFormAttack);
    return true;
}

void SkillMetaFormAttack::onEnd()
{
    if (get() != nullptr)
    {
        get()->removeEffect(EEffect::MetaFormAttack);
    }
}

bool SkillMetaFormProtection::onStart()
{
    if (get() == nullptr) return false;

    get()->addEffect(EEffect::MetaFormProtection);
    return true;
}

void SkillMetaFormProtection::onEnd()
{
    if (get() != nullptr)
    {
        get()->removeEffect(EEffect::MetaFormProtection);
    }
}

bool SkillMetaFormSupport::onStart()
{
    if (get() == nullptr) return false;

    get()->addEffect(EEffect::MetaFormSupport);
    return true;
}

void SkillMetaFormSupport::onEnd()
{
    if (get() != nullptr)
    {
        get()->removeEffect(EEffect::MetaFormSupport);
    }
}

TUniquePtr<SkillBehaviour> SkillFactory::create(ESkill _skill, class ACommonPawn* _pawn, ENetMode _netMode)
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