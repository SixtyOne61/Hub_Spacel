// Fill out your copyright notice in the Description page of Project Settings.


#include "ModuleComponent.h"
#include "Player/SpacelPlayerState.h"
#include "DataAsset/SetupAttributeDataAsset.h"
#include "DataAsset/MetaFormSetupDataAsset.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GameState/SpacelGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Util/SimplyXml.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"
#include "Player/ShipPawn.h"
#include "Mesh/SpacelInstancedMeshComponent.h"

// Sets default values for this component's properties
UModuleComponent::UModuleComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

    createComponent(EmergencyComponent, TEXT("Emergency_00"));
    createComponent(WeaponComponent, TEXT("Weapon_00"));
    createComponent(ProtectionComponent, TEXT("Protection_00"));
    createComponent(SupportComponent, TEXT("Engine_00"));

    MissileMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Missile_00"));
    if (!ensure(MissileMeshComponent != nullptr)) return;
    MissileMeshComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
}

// Called when the game starts
void UModuleComponent::BeginPlay()
{
	Super::BeginPlay();

    if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        ASpacelGameState* spacelGameState{ Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())) };
        if (spacelGameState != nullptr)
        {
            spacelGameState->OnChangeStateDelegate.AddDynamic(this, &UModuleComponent::OnChangeState);
        }
    }

    // setup events to update count voxel on component
    if (this->SupportComponent != nullptr)
    {
        this->SupportComponent->OnUpdateCountDelegate.AddDynamic(this, &UModuleComponent::OnUpdateCountSupport);
    }

    if (this->ProtectionComponent != nullptr)
    {
        this->ProtectionComponent->OnUpdateCountDelegate.AddDynamic(this, &UModuleComponent::OnUpdateCountProtection);
    }

    if (this->EmergencyComponent != nullptr)
    {
        this->EmergencyComponent->OnUpdateCountDelegate.AddDynamic(this, &UModuleComponent::OnUpdateCountEmergency);
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

    lb_call(this->EmergencyComponent);
    lb_call(this->WeaponComponent);
    lb_call(this->ProtectionComponent);
    lb_call(this->SupportComponent);

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
    if(_state != EGameState::InGame) return;

    if (APawn* pawn = Cast<APawn>(this->GetOwner()))
    {
        ASpacelPlayerState* spacelPlayerState = pawn->GetPlayerState<ASpacelPlayerState>();
        if (spacelPlayerState == nullptr)
        {
#if WITH_EDITOR
            UseForm(EFormType::Base, true);

            // TO DO
            //lb_readXml(false, this->WeaponDataAsset, this->R_MissileLocations, "Missile");
            //if (this->MissileMeshComponent != nullptr && this->R_MissileLocations.Num() != 0)
            //{
            //    this->MissileMeshComponent->SetRelativeLocation(this->R_MissileLocations[0]);
            //}
#endif // WITH_EDITOR
        }
        else
        {
            activateBonus((ESkill)spacelPlayerState->getSkillId(ESkillType::Low));
            UseForm(EFormType::Base, true);

            // TO DO
            //lb_readXml(mediumSkillId == (uint8)ESkill::Missile, this->WeaponDataAsset, R_MissileLocations, "Missile");
            //if (this->MissileMeshComponent != nullptr && this->R_MissileLocations.Num() != 0)
            //{
            //    this->MissileMeshComponent->SetRelativeLocation(this->R_MissileLocations[0]);
            //}
        }
    }
}

void UModuleComponent::OnUpdateCountSupport(TArray<FVector_NetQuantize> const& _locations, int32 _max)
{
    if (AShipPawn* pawn = Cast<AShipPawn>(this->GetOwner()))
    {
        pawn->setLocationExhaustFx(_locations);
    }

    this->OnUpdateCountSupportDelegate.Broadcast(_locations.Num(), _max);
}

void UModuleComponent::OnUpdateCountProtection(TArray<FVector_NetQuantize> const& _locations, int32 _max)
{
    int32 curr = _locations.Num();
    int32 max = _max;

    if (this->EmergencyComponent != nullptr)
    {
        curr += this->EmergencyComponent->GetNum();
        max += this->EmergencyComponent->GetMax();
    }

    this->OnUpdateCountProtectionDelegate.Broadcast(curr, max);
}

void UModuleComponent::OnUpdateCountEmergency(TArray<FVector_NetQuantize> const& _locations, int32 _max)
{
    int32 curr = _locations.Num();
    int32 max = _max;

    if (this->ProtectionComponent != nullptr)
    {
        curr += this->ProtectionComponent->GetNum();
        max += this->ProtectionComponent->GetMax();
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

    lb(this->ProtectionComponent);
    lb(this->EmergencyComponent);
    lb(this->SupportComponent);
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

    lb(this->WeaponComponent);
    lb(this->ProtectionComponent);
    lb(this->EmergencyComponent);
    lb(this->SupportComponent);
    m_activatedForms.Empty();
}

void UModuleComponent::restarted()
{
    UseForm(EFormType::Base, true);
}

ESkillReturn UModuleComponent::onSwapEmergency(uint32 _nbMatiereUseForOne)
{
    AShipPawn* pawn = Cast<AShipPawn>(this->GetOwner());
    if(pawn == nullptr) return ESkillReturn::InternError;

    // check component
    if(this->EmergencyComponent == nullptr) return ESkillReturn::InternError;
    if(this->ProtectionComponent == nullptr) return ESkillReturn::InternError;

    int32 nbToRepair = this->EmergencyComponent->GetNbRemoved();
    int32 nbMatiereNeeded = nbToRepair * _nbMatiereUseForOne;
    int32 nbCanRepair = pawn->RU_Matiere / _nbMatiereUseForOne;
    int32 nbToSwap = nbToRepair - nbCanRepair;

    // if we can't repair swapped element
    if(this->ProtectionComponent->GetNum() < nbToSwap) return ESkillReturn::Unavailable;

    this->EmergencyComponent->Repair(nbToRepair);
    pawn->addMatiere(-1 * nbCanRepair * _nbMatiereUseForOne, EMatiereOrigin::Lost);

    this->ProtectionComponent->RemoveRandom(nbToSwap);

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

    lb(this->WeaponComponent, ESkill::FireRate);
    lb(this->ProtectionComponent, ESkill::HeavyProtection);
    lb(this->SupportComponent, ESkill::Speedy);
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
    }

    return EFormType::Base;
}

void UModuleComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UModuleComponent, R_MissileLocations);
}