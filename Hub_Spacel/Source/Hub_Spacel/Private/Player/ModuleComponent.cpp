// Fill out your copyright notice in the Description page of Project Settings.


#include "ModuleComponent.h"
#include "Player/SpacelPlayerState.h"
#include "DataAsset/SetupAttributeDataAsset.h"
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
    auto lb_readXml = [](bool _isHeavy, USetupAttributeDataAsset* _dataAsset, TArray<FVector>& _out, FString&& _name)
    {
        if (!ensure(_dataAsset != nullptr)) return;
        FString const& path{ _isHeavy ? _dataAsset->HeavyPath : _dataAsset->DefaultPath };

        SimplyXml::FContainer<FVector> locationInformation{ _name };
        SimplyXml::FReader reader{ FPaths::ProjectDir() + path };
        reader.read(locationInformation);

        _out = std::move(locationInformation.Values);
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

    auto lb_readXml = [](bool _isHeavy, USetupAttributeDataAsset* _dataAsset, TArray<FVector>& _out, FString && _name)
    {
        if (!ensure(_dataAsset != nullptr)) return;
        FString const& path{ _isHeavy ? _dataAsset->HeavyPath : _dataAsset->DefaultPath };

        SimplyXml::FContainer<FVector> locationInformation{ _name };
        SimplyXml::FReader reader{ FPaths::ProjectDir() + path };
        reader.read(locationInformation);

        _out = std::move(locationInformation.Values);
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

void UModuleComponent::buildShip(UInstancedStaticMeshComponent*& _mesh, UStaticMeshDataAsset* _staticMesh, TArray<FVector> const& _locations)
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
    auto lb = [](TArray<FVector>& _out, TArray<FVector>& _in)
    {
        _out.Append(_in);
        _in.Empty();
    };

    lb(this->R_RemovedProtectionLocations, this->RU_ProtectionLocations);
    lb(this->R_RemovedSupportLocations, this->RU_SupportLocations);
    lb(this->R_RemovedAttackLocations, this->RU_AttackLocations);

    OnRep_Attack();
    OnRep_Protection();
    OnRep_Support();
}

void UModuleComponent::restarted()
{
    auto lb = [](TArray<FVector>& _out, TArray<FVector>& _in)
    {
        _out.Append(_in);
        _in.Empty();
    };

    lb(this->RU_ProtectionLocations, this->R_RemovedProtectionLocations);
    lb(this->RU_SupportLocations, this->R_RemovedSupportLocations);
    lb(this->RU_AttackLocations, this->R_RemovedAttackLocations);

    OnRep_Attack();
    OnRep_Protection();
    OnRep_Support();
}

float UModuleComponent::getPercentProtection() const
{
    return (float)this->RU_ProtectionLocations.Num() / ((float)(this->RU_ProtectionLocations.Num() + this->R_RemovedProtectionLocations.Num()));
}

float UModuleComponent::getPercentSupport() const
{
    return (float)this->RU_SupportLocations.Num() / ((float)(this->RU_SupportLocations.Num() + this->R_RemovedSupportLocations.Num()));
}

void UModuleComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UModuleComponent, RU_AttackLocations);
    DOREPLIFETIME(UModuleComponent, RU_ProtectionLocations);
    DOREPLIFETIME(UModuleComponent, RU_SupportLocations);
    DOREPLIFETIME(UModuleComponent, R_RemovedProtectionLocations);
    DOREPLIFETIME(UModuleComponent, R_RemovedSupportLocations);
    DOREPLIFETIME(UModuleComponent, R_RemovedAttackLocations);
    DOREPLIFETIME(UModuleComponent, R_MissileLocations);
}