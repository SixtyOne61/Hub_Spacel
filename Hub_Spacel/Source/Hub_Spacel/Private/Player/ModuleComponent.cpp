// Fill out your copyright notice in the Description page of Project Settings.


#include "ModuleComponent.h"
#include "Player/SpacelPlayerState.h"
#include "GameState/SpacelGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Util/SimplyXml.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "Player/ShipPawn.h"
#include "Mesh/SpacelInstancedMeshComponent.h"
#include "Mesh/EmergencyInstancedMeshComponent.h"

// Sets default values for this component's properties
UModuleComponent::UModuleComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
}

// Called when the game starts
void UModuleComponent::BeginPlay()
{
	Super::BeginPlay();

    ACommonPawn* pawn = getPawn<ACommonPawn>();
    if (pawn == nullptr) return;

    if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        ASpacelGameState* spacelGameState{ Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())) };
        if (spacelGameState != nullptr)
        {
            spacelGameState->OnChangeStateDelegate.AddDynamic(this, &UModuleComponent::OnChangeState);
        }

        pawn->MissileComponent->Read();
        pawn->MissileComponent->InitLocations(false);
        pawn->MissileComponent->resetBuild();
    }

    // setup events to update count voxel on component
    if (pawn->SupportComponent != nullptr)
    {
        pawn->SupportComponent->OnUpdateCountDelegate.AddDynamic(this, &UModuleComponent::OnUpdateCountSupport);
    }

    if (pawn->ProtectionComponent != nullptr)
    {
        pawn->ProtectionComponent->OnUpdateCountDelegate.AddDynamic(this, &UModuleComponent::OnUpdateCountProtection);
    }

    if (pawn->EmergencyComponent != nullptr)
    {
        pawn->EmergencyComponent->OnUpdateCountDelegate.AddDynamic(this, &UModuleComponent::OnUpdateCountEmergency);
    }
}

void UModuleComponent::UseForm(EFormType _type, bool _refresh)
{
    auto lb_call = [&_refresh, &_type](USpacelInstancedMeshComponent* _component)
    {
        if (_component != nullptr)
        {
            _component->UseForm(_type, _refresh);
        }
    };

    ACommonPawn* pawn = getPawn<ACommonPawn>();
    if (pawn == nullptr) return;

    lb_call(pawn->EmergencyComponent);
    lb_call(pawn->WeaponComponent);
    lb_call(pawn->ProtectionComponent);
    lb_call(pawn->SupportComponent);

    m_activatedForms.AddUnique(_type);
}

void UModuleComponent::buildLobbyShip(ESkill _skillId, ESkillType _type)
{
    if(_type == ESkillType::Low)
    {
        activateBonus(_skillId);
    }

    UseForm(getFormType(_skillId), true);
}

void UModuleComponent::OnChangeState(EGameState _state)
{
    if(_state != EGameState::LockPrepare) return;

    if (APawn* pawn = Cast<APawn>(this->GetOwner()))
    {
        ASpacelPlayerState* spacelPlayerState = pawn->GetPlayerState<ASpacelPlayerState>();
        if (spacelPlayerState == nullptr)
        {
#if WITH_EDITOR
            UseForm(EFormType::Base, true);
#endif // WITH_EDITOR
        }
        else
        {
            activateBonus((ESkill)spacelPlayerState->getSkillId(ESkillType::Low));
            UseForm(EFormType::Base, true);
        }
    }
}

void UModuleComponent::OnUpdateCountSupport(TArray<FVector_NetQuantize> const& _locations, int32 _max)
{
    if (AShipPawn* pawn = getPawn<AShipPawn>())
    {
        pawn->setLocationExhaustFx(_locations);
    }

    this->OnUpdateCountSupportDelegate.Broadcast(_locations.Num(), _max);
}

void UModuleComponent::OnUpdateCountProtection(TArray<FVector_NetQuantize> const& _locations, int32 _max)
{
    int32 curr = _locations.Num();
    int32 max = _max;

    ACommonPawn* pawn = getPawn<ACommonPawn>();
    if (pawn == nullptr) return;

    if (pawn->EmergencyComponent != nullptr)
    {
        curr += pawn->EmergencyComponent->GetNum();
        max += pawn->EmergencyComponent->GetMax();
    }

    this->OnUpdateCountProtectionDelegate.Broadcast(curr, max);
}

void UModuleComponent::OnUpdateCountEmergency(TArray<FVector_NetQuantize> const& _locations, int32 _max)
{
    int32 curr = _locations.Num();
    int32 max = _max;

    ACommonPawn* pawn = getPawn<ACommonPawn>();
    if (pawn == nullptr) return;

    if (pawn->ProtectionComponent != nullptr)
    {
        curr += pawn->ProtectionComponent->GetNum();
        max += pawn->ProtectionComponent->GetMax();
    }

    this->OnUpdateCountProtectionDelegate.Broadcast(curr, max);
}

void UModuleComponent::setCollisionProfile(FString _team)
{
    auto lb = [&_team](USpacelInstancedMeshComponent *& _component)
    {
        if (_component != nullptr)
        {
            _component->SetCollisionProfileName(*_team);
        }
    };

    ACommonPawn* pawn = getPawn<ACommonPawn>();
    if (pawn == nullptr) return;

    lb(pawn->ProtectionComponent);
    auto tmp = Cast<USpacelInstancedMeshComponent>(pawn->EmergencyComponent);
    lb(tmp);
    lb(pawn->SupportComponent);
}

void UModuleComponent::kill()
{
    // call clean on all component
    auto lb = [](USpacelInstancedMeshComponent*& _component)
    {
        if (_component != nullptr)
        {
            _component->RPCNetMulticastClean();
        }
    };

    ACommonPawn* pawn = getPawn<ACommonPawn>();
    if (pawn == nullptr) return;

    lb(pawn->WeaponComponent);
    lb(pawn->ProtectionComponent);
    auto tmp = Cast<USpacelInstancedMeshComponent>(pawn->EmergencyComponent);
    lb(tmp);
    lb(pawn->SupportComponent);
    m_activatedForms.Empty();
}

void UModuleComponent::restarted()
{
    UseForm(EFormType::Base, true);
}

ESkillReturn UModuleComponent::onSwapEmergency(uint32 _nbMatiereUseForOne)
{
    AShipPawn* pawn = getPawn<AShipPawn>();
    if (pawn == nullptr) return ESkillReturn::InternError;

    // check component
    if(pawn->EmergencyComponent == nullptr) return ESkillReturn::InternError;
    if(pawn->ProtectionComponent == nullptr) return ESkillReturn::InternError;

    int32 nbToRepair = pawn->EmergencyComponent->GetNbRemoved();
    int32 nbMatiereNeeded = nbToRepair * _nbMatiereUseForOne;
    int32 nbCanRepair = pawn->RU_Matiere / _nbMatiereUseForOne;
    int32 nbToSwap = FMath::Max(nbToRepair - nbCanRepair, 0);

    // if we can't repair swapped element
    if(pawn->ProtectionComponent->GetNum() < nbToSwap) return ESkillReturn::Unavailable;

    pawn->EmergencyComponent->Repair(nbToRepair);
    pawn->addMatiere(-1 * (nbToRepair - nbToSwap) * _nbMatiereUseForOne, EMatiereOrigin::Lost);

    pawn->ProtectionComponent->RemoveRandom(nbToSwap);
    return ESkillReturn::Success;
}

void UModuleComponent::activeMetaForm(EEffect _type)
{
    UseForm(getFormType(_type), false);
}

void UModuleComponent::removeMetaForm(EEffect _type)
{
    auto form = getFormType(_type);
    m_activatedForms.Remove(form);

    m_activatedForms.Num() ? UseForm(m_activatedForms.Pop(), false) : UseForm(EFormType::Base, false);
}

void UModuleComponent::activateBonus(ESkill _skillId)
{
    // we can have bonus only on one component
    auto lb = [&_skillId](USpacelInstancedMeshComponent*& _component, ESkill _compare)
    {
        if (_component != nullptr)
        {
            _component->SetUseBonus(_skillId == _compare);
        }
    };

    ACommonPawn* pawn = getPawn<ACommonPawn>();
    if (pawn == nullptr) return;

    lb(pawn->WeaponComponent, ESkill::FireRate);
    lb(pawn->ProtectionComponent, ESkill::HeavyProtection);
    lb(pawn->SupportComponent, ESkill::Speedy);
}

EFormType UModuleComponent::getFormType(ESkill _skillId) const
{
    switch (_skillId)
    {
    case ESkill::MetaFormAttack:
        return EFormType::MetaFormAttack;
    case ESkill::MetaFormProtection:
        return EFormType::MetaFormProtection;
    case ESkill::MetaFormSupport:
        return EFormType::MetaFormSupport;
    case ESkill::EscapeMode:
        return EFormType::EscapeMode;
    case ESkill::Missile:
        return EFormType::Missile;
    }

    return EFormType::Base;
}

EFormType UModuleComponent::getFormType(EEffect _type) const
{
    switch (_type)
    {
    case EEffect::MetaFormAttack:
        return EFormType::MetaFormAttack;
    case EEffect::MetaFormProtection:
        return EFormType::MetaFormProtection;
    case EEffect::MetaFormSupport:
        return EFormType::MetaFormSupport;
    case EEffect::EscapeMode:
        return EFormType::EscapeMode;
    case EEffect::Missile:
        return EFormType::Missile;
    }

    return EFormType::Base;
}
