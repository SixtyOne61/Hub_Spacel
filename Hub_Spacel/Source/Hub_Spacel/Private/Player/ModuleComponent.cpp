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
            spacelGameState->OnStartGameDelegate.AddDynamic(this, &UModuleComponent::OnStartGame);
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

void UModuleComponent::OnStartGame()
{
    auto lb_readXml = [](uint8 _level, USetupAttributeDataAsset* _dataAsset, TArray<FVector>& _out)
    {
        if (!ensure(_dataAsset != nullptr)) return;
        FString const& path{ _level > 0 ? _dataAsset->HeavyPath : _dataAsset->DefaultPath };

        SimplyXml::FContainer<FVector> locationInformation{ "Location" };
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
            lb_readXml(0, this->WeaponDataAsset, RU_AttackLocations);
            buildShip(this->WeaponMeshComponent, this->WeaponDataAsset, RU_AttackLocations);
            lb_readXml(0, this->ProtectionDataAsset, RU_ProtectionLocations);
            buildShip(this->ProtectionMeshComponent, this->ProtectionDataAsset, RU_ProtectionLocations);
            lb_readXml(0, this->SupportDataAsset, RU_SupportLocations);
            buildShip(this->SupportMeshComponent, this->SupportDataAsset, RU_SupportLocations);
#endif // WITH_EDITOR
        }
        else
        {
            lb_readXml(spacelPlayerState->Attack, this->WeaponDataAsset, this->RU_AttackLocations);
            buildShip(this->WeaponMeshComponent, this->WeaponDataAsset, this->RU_AttackLocations);

            lb_readXml(spacelPlayerState->Protection, this->ProtectionDataAsset, this->RU_ProtectionLocations);
            buildShip(this->ProtectionMeshComponent, this->ProtectionDataAsset, this->RU_ProtectionLocations);

            lb_readXml(spacelPlayerState->Support, this->SupportDataAsset, this->RU_SupportLocations);
            buildShip(this->SupportMeshComponent, this->SupportDataAsset, this->RU_SupportLocations);
            setLocationExhaustFx();

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

void UModuleComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UModuleComponent, RU_AttackLocations);
    DOREPLIFETIME(UModuleComponent, RU_ProtectionLocations);
    DOREPLIFETIME(UModuleComponent, RU_SupportLocations);
    DOREPLIFETIME(UModuleComponent, R_RemovedProtectionLocations);
    DOREPLIFETIME(UModuleComponent, R_RemovedSupportLocations);
}