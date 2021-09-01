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

// Sets default values for this component's properties
UModuleComponent::UModuleComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

    ProtectionMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Protection_00"));
    if (!ensure(ProtectionMeshComponent != nullptr)) return;
    ProtectionMeshComponent->SetRenderCustomDepth(true);
    ProtectionMeshComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);

    WeaponMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Weapon_00"));
    if (!ensure(WeaponMeshComponent != nullptr)) return;
    WeaponMeshComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);

    SupportMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Support_00"));
    if (!ensure(SupportMeshComponent != nullptr)) return;
    SupportMeshComponent->SetRenderCustomDepth(true);
    SupportMeshComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);

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
            spacelGameState->OnChangeStateDelegate.AddDynamic(this, &UModuleComponent::OnStartGame);
        }
    }

    // if we reconnect array will be replicate
    OnRep_Attack();
    OnRep_Protection();
    OnRep_Support();
}

void UModuleComponent::OnRep_Attack()
{
    buildShip(this->WeaponMeshComponent, this->WeaponDataAsset, RU_AttackLocations);
}

void UModuleComponent::OnRep_Protection()
{
    this->OnUpdateCountProtectionDelegate.Broadcast(this->RU_ProtectionLocations.Num(), m_maxProtection);
    buildShip(this->ProtectionMeshComponent, this->ProtectionDataAsset, RU_ProtectionLocations);

    if (AShipPawn* shipPawn = Cast<AShipPawn>(this->GetOwner()))
    {
        if (this->MetaFormDataAsset != nullptr)
        {
            if (this->ProtectionMeshComponent != nullptr)
            {
                // check override
                if (shipPawn->hasEffect(EEffect::MetaFormAttack))
                {
                    this->ProtectionMeshComponent->SetStaticMesh(this->MetaFormDataAsset->AttackStaticMesh);
                }
                else if (shipPawn->hasEffect(EEffect::MetaFormProtection))
                {
                    this->ProtectionMeshComponent->SetStaticMesh(this->MetaFormDataAsset->ProtectionStaticMesh);
                }
                else if (shipPawn->hasEffect(EEffect::MetaFormSupport))
                {
                    this->ProtectionMeshComponent->SetStaticMesh(this->MetaFormDataAsset->SupportStaticMesh);
                }
            }
        }
    }
}

void UModuleComponent::OnRep_Support()
{
    this->OnUpdateCountSupportDelegate.Broadcast(this->RU_SupportLocations.Num(), m_maxSupport);
    buildShip(this->SupportMeshComponent, this->SupportDataAsset, RU_SupportLocations);
    setLocationExhaustFx();
}

void UModuleComponent::SetMax_Implementation(int32 _maxProtection, int32 _maxSupport)
{
    m_maxProtection = _maxProtection;
    m_maxSupport = _maxSupport;

    // force refresh UI
    this->OnUpdateCountProtectionDelegate.Broadcast(this->RU_ProtectionLocations.Num(), m_maxProtection);
    this->OnUpdateCountSupportDelegate.Broadcast(this->RU_SupportLocations.Num(), m_maxSupport);
}

void UModuleComponent::BuildShipLobby()
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
        if (ASpacelPlayerState* spacelPlayerState = pawn->GetPlayerState<ASpacelPlayerState>())
        {
            uint8 lowSkillId = spacelPlayerState->getSkillId(ESkillType::Low);
            uint8 mediumSkillId = spacelPlayerState->getSkillId(ESkillType::Medium);

            lb_readXml(lowSkillId == (uint8)ESkill::FireRate, this->WeaponDataAsset, this->RU_AttackLocations, "Location");
            buildShip(this->WeaponMeshComponent, this->WeaponDataAsset, this->RU_AttackLocations);

            lb_readXml(lowSkillId == (uint8)ESkill::HeavyProtection, this->ProtectionDataAsset, this->RU_ProtectionLocations, "Location");
            buildShip(this->ProtectionMeshComponent, this->ProtectionDataAsset, this->RU_ProtectionLocations);

            lb_readXml(lowSkillId == (uint8)ESkill::Speedy, this->SupportDataAsset, this->RU_SupportLocations, "Location");
            buildShip(this->SupportMeshComponent, this->SupportDataAsset, this->RU_SupportLocations);
            setLocationExhaustFx();

            lb_readXml(mediumSkillId == (uint8)ESkill::Missile, this->WeaponDataAsset, R_MissileLocations, "Missile");
            if (this->MissileMeshComponent != nullptr && this->R_MissileLocations.Num() != 0)
            {
                this->MissileMeshComponent->SetRelativeLocation(this->R_MissileLocations[0]);
            }

            this->SetMax(this->RU_ProtectionLocations.Num(), this->RU_SupportLocations.Num());
        }
    }
}

void UModuleComponent::OnStartGame(EGameState _state)
{
    if(_state != EGameState::InGame) return;

    auto lb_readXml = [](bool _isHeavy, USetupAttributeDataAsset* _dataAsset, TArray<FVector_NetQuantize>& _out, FString && _name)
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
        ASpacelPlayerState* spacelPlayerState = pawn->GetPlayerState<ASpacelPlayerState>();
        if (spacelPlayerState == nullptr)
        {
#if WITH_EDITOR
            lb_readXml(false, this->WeaponDataAsset, RU_AttackLocations, "Location");
            buildShip(this->WeaponMeshComponent, this->WeaponDataAsset, RU_AttackLocations);
            lb_readXml(false, this->ProtectionDataAsset, RU_ProtectionLocations, "Location");
            buildShip(this->ProtectionMeshComponent, this->ProtectionDataAsset, RU_ProtectionLocations);
            lb_readXml(false, this->SupportDataAsset, RU_SupportLocations, "Location");
            buildShip(this->SupportMeshComponent, this->SupportDataAsset, RU_SupportLocations);

            lb_readXml(false, this->WeaponDataAsset, this->R_MissileLocations, "Missile");
            if (this->MissileMeshComponent != nullptr && this->R_MissileLocations.Num() != 0)
            {
                this->MissileMeshComponent->SetRelativeLocation(this->R_MissileLocations[0]);
            }
#endif // WITH_EDITOR
        }
        else
        {
            uint8 lowSkillId = spacelPlayerState->getSkillId(ESkillType::Low);
            uint8 mediumSkillId = spacelPlayerState->getSkillId(ESkillType::Medium);

            lb_readXml(lowSkillId == (uint8)ESkill::FireRate, this->WeaponDataAsset, this->RU_AttackLocations, "Location");
            buildShip(this->WeaponMeshComponent, this->WeaponDataAsset, this->RU_AttackLocations);

            lb_readXml(lowSkillId == (uint8)ESkill::HeavyProtection, this->ProtectionDataAsset, this->RU_ProtectionLocations, "Location");
            lb_readXml(lowSkillId == (uint8)ESkill::HeavyProtection, this->ProtectionDataAsset, this->EmergencyLocations, "Emergency");
            this->RU_ProtectionLocations.Append(this->EmergencyLocations);
            buildShip(this->ProtectionMeshComponent, this->ProtectionDataAsset, this->RU_ProtectionLocations);

            lb_readXml(lowSkillId == (uint8)ESkill::Speedy, this->SupportDataAsset, this->RU_SupportLocations, "Location");
            buildShip(this->SupportMeshComponent, this->SupportDataAsset, this->RU_SupportLocations);
            setLocationExhaustFx();

            // add effect for low skill
            if (AShipPawn* shipPawn = Cast<AShipPawn>(pawn))
            {
                switch (lowSkillId)
                {
                case (uint8)ESkill::FireRate:
                    shipPawn->addEffect(EEffect::SkillPassiveFireRate);
                    break;
                case (uint8)ESkill::HeavyProtection:
                    shipPawn->addEffect(EEffect::SkillPassiveProtection);
                    break;
                case (uint8)ESkill::Speedy:
                    shipPawn->addEffect(EEffect::SkillPassiveSpeed);
                    break;
                }
            }

            lb_readXml(mediumSkillId == (uint8)ESkill::Missile, this->WeaponDataAsset, R_MissileLocations, "Missile");
            if (this->MissileMeshComponent != nullptr && this->R_MissileLocations.Num() != 0)
            {
                this->MissileMeshComponent->SetRelativeLocation(this->R_MissileLocations[0]);
            }

            this->SetMax(this->RU_ProtectionLocations.Num(), this->RU_SupportLocations.Num());
        }
    }
}

void UModuleComponent::buildShip(UInstancedStaticMeshComponent*& _mesh, UStaticMeshDataAsset* _staticMesh, TArray<FVector_NetQuantize> const& _locations)
{
    if (_mesh && _staticMesh)
    {
        _mesh->ClearInstances();
        _mesh->SetStaticMesh(_staticMesh->StaticMesh);
        _mesh->SetEnableGravity(false);

        for (auto const& _location : _locations)
        {
            FTransform voxelTransform{};
            voxelTransform.SetLocation(_location);
            _mesh->AddInstance(voxelTransform);
        }
    }
}

void UModuleComponent::setLocationExhaustFx()
{
    if (AShipPawn* pawn = Cast<AShipPawn>(this->GetOwner()))
    {
        pawn->setLocationExhaustFx(this->RU_SupportLocations);
    }
}

void UModuleComponent::setCollisionProfile(FString _team)
{
    auto lb = [&_team](UInstancedStaticMeshComponent *& _component)
    {
        if (_component != nullptr)
        {
            _component->SetCollisionProfileName(*_team);
        }
    };

    lb(this->ProtectionMeshComponent);
    lb(this->WeaponMeshComponent);
    lb(this->SupportMeshComponent);
}

void UModuleComponent::kill()
{
    auto lb = [](TArray<FVector_NetQuantize>& _out, TArray<FVector_NetQuantize>& _in)
    {
        _out.Append(_in);
        _in.Empty();
    };

    lb(this->RemovedProtectionLocations, this->RU_ProtectionLocations);
    lb(this->RemovedSupportLocations, this->RU_SupportLocations);
    lb(this->RemovedAttackLocations, this->RU_AttackLocations);

    OnRep_Attack();
    OnRep_Protection();
    OnRep_Support();
}

void UModuleComponent::restarted()
{
    auto lb = [](TArray<FVector_NetQuantize>& _out, TArray<FVector_NetQuantize>& _in)
    {
        _out.Append(_in);
        _in.Empty();
    };

    lb(this->RU_ProtectionLocations, this->RemovedProtectionLocations);
    lb(this->RU_SupportLocations, this->RemovedSupportLocations);
    lb(this->RU_AttackLocations, this->RemovedAttackLocations);

    OnRep_Attack();
    OnRep_Protection();
    OnRep_Support();
}

float UModuleComponent::getPercentProtection() const
{
    return (float)this->RU_ProtectionLocations.Num() / m_maxProtection;
}

float UModuleComponent::getPercentSupport() const
{
    return (float)this->RU_SupportLocations.Num() / m_maxSupport;
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
    if (!ensure(this->MetaFormDataAsset != nullptr)) return;

    FString tag {};
    UStaticMesh* mesh { nullptr };
    if (_type == EEffect::MetaFormProtection)
    {
        tag = "Protection";
        mesh = this->MetaFormDataAsset->ProtectionStaticMesh;
    }
    else if (_type == EEffect::MetaFormAttack)
    {
        tag = "Attack";
        mesh = this->MetaFormDataAsset->AttackStaticMesh;
    }
    else if (_type == EEffect::MetaFormSupport)
    {
        tag = "Support";
        mesh = this->MetaFormDataAsset->SupportStaticMesh;
    }

    FString const& path { this->MetaFormDataAsset->DefaultPath };

    TArray<FVector_NetQuantize> out{};

    auto lbread = [&out, &path](FString const& _tag)
    {
        SimplyXml::FContainer<FVector> locationInformation{ _tag };
        SimplyXml::FReader reader{ FPaths::ProjectDir() + path };
        reader.read(locationInformation);

        for (FVector loc : locationInformation.Values)
        {
            out.Add(loc);
        }
    };

    lbread("Emergency");
    lbread(tag);

    auto lb = [&out](TArray<FVector_NetQuantize>& _fill, int32 _max)
    {
        _fill.Empty();
        while (_max != 0 && out.Num() != 0)
        {
            _fill.Add(out.Pop());
            --_max;
        }
    };

    lb(this->RU_ProtectionLocations, this->RU_ProtectionLocations.Num());
    lb(this->RemovedProtectionLocations, this->RemovedProtectionLocations.Num());

    OnRep_Protection();
}

void UModuleComponent::removeMetaForm()
{
    if (APawn* pawn = Cast<APawn>(this->GetOwner()))
    {
        if(ASpacelPlayerState* spacelPlayerState = pawn->GetPlayerState<ASpacelPlayerState>())
        {
            uint8 lowSkillId = spacelPlayerState->getSkillId(ESkillType::Low);
            bool isHeavy = lowSkillId == (uint8)ESkill::HeavyProtection;

            if (!ensure(this->ProtectionDataAsset != nullptr)) return;
            FString const& path { isHeavy ? this->ProtectionDataAsset->HeavyPath : this->ProtectionDataAsset->DefaultPath };

            TArray<FVector_NetQuantize> out{};
            auto lbread = [&out, &path](FString const& _tag)
            {
                SimplyXml::FContainer<FVector> locationInformation{ _tag };
                SimplyXml::FReader reader{ FPaths::ProjectDir() + path };
                reader.read(locationInformation);

                for (FVector loc : locationInformation.Values)
                {
                    out.Add(loc);
                }
            };

            lbread("Emergency");
            lbread("Location");

            int32 maxProtection = this->RU_ProtectionLocations.Num();
            int32 maxRemove = this->RemovedProtectionLocations.Num();

            auto lb = [&out](TArray<FVector_NetQuantize>& _fill, int32 _max)
            {
                _fill.Empty();
                while (_max != 0 && out.Num() != 0)
                {
                    _fill.Add(out.Pop());
                    --_max;
                }
            };

            lb(this->RU_ProtectionLocations, this->RU_ProtectionLocations.Num());
            lb(this->RemovedProtectionLocations, this->RemovedProtectionLocations.Num());

            OnRep_Protection();
        }
    }
}

void UModuleComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UModuleComponent, RU_AttackLocations);
    DOREPLIFETIME(UModuleComponent, RU_ProtectionLocations);
    DOREPLIFETIME(UModuleComponent, RU_SupportLocations);
    DOREPLIFETIME(UModuleComponent, R_MissileLocations);
}