// Fill out your copyright notice in the Description page of Project Settings.


#include "SpacelPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Player/ShipPawn.h"

void ASpacelPlayerState::BeginPlay()
{
    Super::BeginPlay();
#if WITH_EDITOR
    if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        static int idTeam{ 0 };
        idTeam = (idTeam) % 2 + 1;
        SetTeam("Team " + FString::FromInt(idTeam));
    }
#endif
}

void ASpacelPlayerState::RPCServerAddSkill_Implementation(ESkill _id, ESkillType _type)
{
    switch (_type)
    {
        case ESkillType::Low: R_LowSkill = (uint8)_id; break;
        case ESkillType::Medium: R_MediumSkill = (uint8)_id; break;
        case ESkillType::Hight: R_HightSkill = (uint8)_id; break;
    }
}

void ASpacelPlayerState::SetTeam(FString const& _team)
{
    this->R_Team = _team;

    FTimerHandle handle {};
    this->GetWorldTimerManager().SetTimer(handle, this, &ASpacelPlayerState::WaitPawnCreation, 1.0f, false, 1.0f);
}

void ASpacelPlayerState::WaitPawnCreation()
{
    if (AShipPawn* shipPawn = this->GetPawn<AShipPawn>())
    {
        shipPawn->setCollisionProfile(this->R_Team);
        shipPawn->Team = *(this->R_Team);
        FString tag = "Team:" + this->R_Team;
        shipPawn->Tags.Add(*tag);
    }
    else
    {
        FTimerHandle handle{};
        this->GetWorldTimerManager().SetTimer(handle, this, &ASpacelPlayerState::WaitPawnCreation, 1.0f, false, 1.0f);
    }
}

uint8 ASpacelPlayerState::getSkillId(ESkillType _type) const
{
    switch (_type)
    {
    case ESkillType::Low: return R_LowSkill;
    case ESkillType::Medium: return R_MediumSkill;
    case ESkillType::Hight: return R_HightSkill;
    }

    ensure(false);
    return -1;
}

void ASpacelPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASpacelPlayerState, R_Team);
    DOREPLIFETIME(ASpacelPlayerState, R_LowSkill);
    DOREPLIFETIME(ASpacelPlayerState, R_MediumSkill);
    DOREPLIFETIME(ASpacelPlayerState, R_HightSkill);
    DOREPLIFETIME(ASpacelPlayerState, R_KilledByPlayerId);
}
