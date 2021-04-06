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

void ASpacelPlayerState::setRemainingSkillPoint(uint8 && _val)
{
    this->RemainingSkillPoint = _val;
    this->OnUpdateRemainingSkillPointDelegate.Broadcast();
}

uint8 ASpacelPlayerState::getSkillPoint(ESkillType const& _type) const
{
    switch (_type)
    {
        case ESkillType::Attack: return this->R_Attack;
        case ESkillType::Protection: return this->R_Protection;
        case ESkillType::Support: return this->R_Support;
        default: ensure(true); return 0;
    }
}

void ASpacelPlayerState::RPCSetSkillPoint_Implementation(ESkillType const& _type, uint8 _value)
{
    switch (_type)
    {
        case ESkillType::Attack: this->R_Attack = _value; return;
        case ESkillType::Protection: this->R_Protection = _value; return;
        case ESkillType::Support: this->R_Support = _value; return;
        default: ensure(true); return;
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

void ASpacelPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ASpacelPlayerState, R_Team);
    DOREPLIFETIME(ASpacelPlayerState, R_Attack);
    DOREPLIFETIME(ASpacelPlayerState, R_Protection);
    DOREPLIFETIME(ASpacelPlayerState, R_Support);
}
