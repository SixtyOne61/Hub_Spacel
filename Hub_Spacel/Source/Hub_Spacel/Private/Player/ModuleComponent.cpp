// Fill out your copyright notice in the Description page of Project Settings.


#include "ModuleComponent.h"
#include "Player/SpacelPlayerState.h"
#include "DataAsset/SetupAttributeDataAsset.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GameState/SpacelGameState.h"
#include "Kismet/GameplayStatics.h"
#include "Util/SimplyXml.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UModuleComponent::UModuleComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

    ProtectionMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Protection_00"));
    if (!ensure(ProtectionMeshComponent != nullptr)) return;
    ProtectionMeshComponent->SetCollisionProfileName("Player");
    ProtectionMeshComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);

    WeaponMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Weapon_00"));
    if (!ensure(WeaponMeshComponent != nullptr)) return;
    WeaponMeshComponent->SetCollisionProfileName("Player");
    WeaponMeshComponent->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);

    SupportMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Support_00"));
    if (!ensure(SupportMeshComponent != nullptr)) return;
    SupportMeshComponent->SetCollisionProfileName("Player");
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
	
}

// Called every frame
void UModuleComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UModuleComponent::OnRep_Attack()
{
    buildShip(this->WeaponMeshComponent, this->WeaponDataAsset, RU_AttackLocations);
}

void UModuleComponent::OnRep_Protection()
{
    buildShip(this->ProtectionMeshComponent, this->ProtectionDataAsset, RU_ProtectionLocations);
}

void UModuleComponent::OnRep_Support()
{
    buildShip(this->SupportMeshComponent, this->SupportDataAsset, RU_SupportLocations);
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
            lb_readXml(spacelPlayerState->Attack, this->WeaponDataAsset, RU_AttackLocations);
            buildShip(this->WeaponMeshComponent, this->WeaponDataAsset, RU_AttackLocations);

            lb_readXml(spacelPlayerState->Protection, this->ProtectionDataAsset, RU_ProtectionLocations);
            buildShip(this->ProtectionMeshComponent, this->ProtectionDataAsset, RU_ProtectionLocations);

            lb_readXml(spacelPlayerState->Support, this->SupportDataAsset, RU_SupportLocations);
            buildShip(this->SupportMeshComponent, this->SupportDataAsset, RU_SupportLocations);
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

void UModuleComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UModuleComponent, RU_AttackLocations);
    DOREPLIFETIME(UModuleComponent, RU_ProtectionLocations);
    DOREPLIFETIME(UModuleComponent, RU_SupportLocations);
}