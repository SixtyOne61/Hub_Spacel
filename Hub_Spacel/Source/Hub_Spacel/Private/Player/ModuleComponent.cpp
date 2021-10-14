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
    auto lb_readXml = [](bool _isHeavy, USetupAttributeDataAsset* _dataAsset, TArray<FVector_NetQuantize>& _out, FString&& _name)
    {
        if (!ensure(_dataAsset != nullptr)) return;
        FString const& path{ _isHeavy ? _dataAsset->HeavyPath : _dataAsset->DefaultPath };

        SimplyXml::FContainer<FVector> locationInformation{ _name };
        SimplyXml::FReader reader{ FPaths::ProjectDir() + path };
        reader.read(locationInformation);

        _out.Empty();
        for (FVector loc : locationInformation.Values)
        {
            _out.Add(loc);
        }
    };

    if (APawn* pawn = Cast<APawn>(this->GetOwner()))
    {
        switch (_type)
        {
            case ESkillType::Low:
                lb_readXml(_skillId == ESkill::FireRate, this->WeaponDataAsset, this->RU_AttackLocations, "Location");
                buildShip(this->WeaponMeshComponent, this->WeaponDataAsset, this->RU_AttackLocations);

                lb_readXml(_skillId == ESkill::HeavyProtection, this->ProtectionDataAsset, this->RU_ProtectionLocations, "Location");
                buildShip(this->ProtectionMeshComponent, this->ProtectionDataAsset, this->RU_ProtectionLocations);

                lb_readXml(_skillId == ESkill::Speedy, this->SupportDataAsset, this->RU_SupportLocations, "Location");
                buildShip(this->SupportMeshComponent, this->SupportDataAsset, this->RU_SupportLocations);
                setLocationExhaustFx();

                this->SetMax(this->RU_ProtectionLocations.Num(), this->RU_SupportLocations.Num());
            break;
        }
    }
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
            ESkill lowSkillId = (ESkill)spacelPlayerState->getSkillId(ESkillType::Low);

            switch (lowSkillId)
            {
                case ESkill::FireRate:
                    if (this->WeaponComponent != nullptr) this->WeaponComponent->SetUseBonus(true);
                    break;
                case ESkill::HeavyProtection:
                    if (this->ProtectionComponent != nullptr) this->ProtectionComponent->SetUseBonus(true);
                    break;
                case ESkill::Speedy:
                    if (this->SupportComponent != nullptr) this->SupportComponent->SetUseBonus(true);
                    break;
            }

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

ESkillReturn UModuleComponent::onSwapEmergency(uint32 _value, uint8 _tresholdPercent)
{
    AShipPawn* pawn = Cast<AShipPawn>(this->GetOwner());
    if(pawn == nullptr) return ESkillReturn::InternError;

    while (this->EmergencyLocationsRemove.Num())
    {
        FVector const& location = this->EmergencyLocationsRemove.Pop();

        for (int i { 0 }; i < this->RemovedProtectionLocations.Num(); ++i)
        {
            if (location == this->RemovedProtectionLocations[i])
            {
                bool canRepair { false };
                int32 nbProtections { RU_ProtectionLocations.Num() };
                int32 nbRemovedProtections { RemovedProtectionLocations.Num() };

                // use matiere first
                if (pawn->RU_Matiere >= (int16)_value)
                {
                    pawn->addMatiere(-1 * _value, EMatiereOrigin::Lost);
                    canRepair = true;
                }
                // then use protection
                else if ((float)nbProtections / (float)(nbProtections + nbRemovedProtections) >= (float)_tresholdPercent / 100.0f)
                {
                    canRepair = true;
                    for (int j { 0 }; j < this->RU_ProtectionLocations.Num(); ++j)
                    {
                        if (!this->EmergencyLocations.Find(this->RU_ProtectionLocations[j]))
                        {
                            this->RemovedProtectionLocations.Add(this->RU_ProtectionLocations[j]);
                            this->RU_ProtectionLocations.RemoveAt(j);
                            break;
                        }
                    }
                }
                else
                {
                    this->EmergencyLocationsRemove.Add(location);
                    OnRep_Protection();
                    return ESkillReturn::Unavailable;
                }

                if (canRepair)
                {
                    this->RemovedProtectionLocations.RemoveAt(i);
                    this->RU_ProtectionLocations.Add(location);
                }
                break;
            }
        }
    }

    OnRep_Protection();
    return ESkillReturn::Success;
}

void UModuleComponent::activeMetaForm(EEffect _type)
{
    switch (_type)
    {
        case EEffect::MetaFormAttack:
            UseForm(EFormType::MetaFormAttack, false);
        break;

        case EEffect::MetaFormProtection:
            UseForm(EFormType::MetaFormProtection, false);
        break;

        case EEffect::MetaFormSupport:
            UseForm(EFormType::MetaFormSupport, false);
        break;

        case EEffect::EscapeMode:
            UseForm(EFormType::EscapeMode, false);
        break;
    }
}

void UModuleComponent::removeMetaForm(EEffect _type)
{
    switch (_type)
    {
    case EEffect::MetaFormAttack:
        m_activatedForms.Remove(EFormType::MetaFormAttack);
        break;

    case EEffect::MetaFormProtection:
        m_activatedForms.Remove(EFormType::MetaFormProtection);
        break;

    case EEffect::MetaFormSupport:
        m_activatedForms.Remove(EFormType::MetaFormSupport);
        break;

    case EEffect::EscapeMode:
        m_activatedForms.Remove(EFormType::EscapeMode);
        break;
    }

    m_activatedForms.Num() ? UseForm(m_activatedForms.Pop(), false) : UseForm(EFormType::Base, false);
}

void UModuleComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UModuleComponent, R_MissileLocations);
}