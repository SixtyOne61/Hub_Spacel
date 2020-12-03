// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipPawn.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DataAsset/StaticMeshDataAsset.h"
#include "DataAsset/SetupAttributeDataAsset.h"
#include "DataAsset/PlayerDataAsset.h"
#include "Player/SpacelPlayerState.h"
#include "GameState/SpacelGameState.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Util/SimplyXml.h"

// Sets default values
AShipPawn::AShipPawn()
{
    // Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    bAlwaysRelevant = true;

    DriverMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Driver_00"));
    if (!ensure(DriverMeshComponent != nullptr)) return;
    RootComponent = DriverMeshComponent;

    BaseShipMeshComponent = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("ShipBase_00"));
    if (!ensure(BaseShipMeshComponent != nullptr)) return;
    BaseShipMeshComponent->SetupAttachment(RootComponent);

    RedZoneMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("RedZone_00"));
    if (!ensure(RedZoneMeshComponent != nullptr)) return;
    RedZoneMeshComponent->OnComponentHit.AddDynamic(this, &AShipPawn::OnComponentHitRedZone);
    RedZoneMeshComponent->ComponentTags.Add("RedZoneComponent");
    RedZoneMeshComponent->SetupAttachment(BaseShipMeshComponent);

    ProtectionMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Protection_00"));
    if (!ensure(ProtectionMeshComponent != nullptr)) return;
    ProtectionMeshComponent->OnComponentHit.AddDynamic(this, &AShipPawn::OnComponentHitProtection);
    ProtectionMeshComponent->ComponentTags.Add("ProtectionComponent");
    ProtectionMeshComponent->SetupAttachment(BaseShipMeshComponent);

    WeaponMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Weapon_00"));
    if (!ensure(WeaponMeshComponent != nullptr)) return;
    WeaponMeshComponent->ComponentTags.Add("AttackComponent");
    WeaponMeshComponent->SetupAttachment(BaseShipMeshComponent);

    SupportMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Support_00"));
    if (!ensure(SupportMeshComponent != nullptr)) return;
    SupportMeshComponent->OnComponentHit.AddDynamic(this, &AShipPawn::OnComponentHitSupport);
    SupportMeshComponent->ComponentTags.Add("SupportComponent");
    SupportMeshComponent->SetupAttachment(BaseShipMeshComponent);

    // Create a spring arm component
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm_00"));
    if (!ensure(SpringArmComponent != nullptr)) return;
    SpringArmComponent->SetupAttachment(BaseShipMeshComponent);

    // Create camera component 
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera_00"));
    if (!ensure(CameraComponent != nullptr)) return;
    CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName); // Attach the camera
}

// Called when the game starts or when spawned
void AShipPawn::BeginPlay()
{
	Super::BeginPlay();

    if (this->HasAuthority())
    {
        ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld()));
        if (spacelGameState != nullptr)
        {
            spacelGameState->OnStartGameDelegate.AddDynamic(this, &AShipPawn::StartGame);
        }
    }

    // reset mesh
    if (this->RedZoneMeshComponent) this->RedZoneMeshComponent->ClearInstances();
    if (this->ProtectionMeshComponent) this->ProtectionMeshComponent->ClearInstances();
    if (this->WeaponMeshComponent) this->WeaponMeshComponent->ClearInstances();
    if (this->SupportMeshComponent) this->SupportMeshComponent->ClearInstances();
}

// Called every frame
void AShipPawn::Tick(float _deltaTime)
{
	Super::Tick(_deltaTime);

    if (this->HasAuthority())
    {
        RPCServerMove(_deltaTime);

        fire(_deltaTime);
    }
}

void AShipPawn::OnRep_PercentFlightAttitude()
{
    if (!ensure(this->DriverMeshComponent != nullptr)) return;
    if (!ensure(this->PlayerDataAsset != nullptr)) return;

    FVector dir = this->DriverMeshComponent->GetForwardVector() * this->RU_PercentFlightAttitude * this->PlayerDataAsset->FlightAttitudeSpeed;
    dir = FMath::Lerp(FVector::ZeroVector, dir, 0.1f);

    this->DriverMeshComponent->AddTorqueInDegrees(dir, NAME_None, true);
}

void AShipPawn::OnRep_PercentTurn()
{
    if (!ensure(this->DriverMeshComponent != nullptr)) return;
    if (!ensure(this->PlayerDataAsset != nullptr)) return;

    FVector dir = this->DriverMeshComponent->GetUpVector() * this->RU_PercentTurn * this->PlayerDataAsset->TurnSpeed;
    dir = FMath::Lerp(FVector::ZeroVector, dir, 0.1f);

    this->DriverMeshComponent->AddTorqueInDegrees(dir, NAME_None, true);
}

void AShipPawn::OnRep_PercentUp()
{
    if (!ensure(this->DriverMeshComponent != nullptr)) return;
    if (!ensure(this->PlayerDataAsset != nullptr)) return;

    FVector dir = this->DriverMeshComponent->GetRightVector() * this->RU_PercentUp * this->PlayerDataAsset->UpSpeed;
    dir = FMath::Lerp(FVector::ZeroVector, dir, 0.1f);

    this->DriverMeshComponent->AddTorqueInDegrees(dir, NAME_None, true);
}

void AShipPawn::addVoxel(class UInstancedStaticMeshComponent* & _mesh, FVector&& _location) const
{
    FTransform voxelTransform{};
    voxelTransform.SetLocation(std::move(_location));
    _mesh->AddInstance(voxelTransform);
}

void AShipPawn::addVoxel(class UInstancedStaticMeshComponent* & _mesh, TArray<FVector>&& _locations) const
{
    for (auto&& location : _locations)
    {
        addVoxel(_mesh, std::forward<FVector>(location));
    }
}

void AShipPawn::RPCClientAddVoxel_Implementation(TArray<FVector> const& _redZoneLocations, TArray<FVector> const& _attackLocations, TArray<FVector> const& _protectionLocations, TArray<FVector> const& _supportLocations)
{
    auto lb_call = [&](UInstancedStaticMeshComponent*& _mesh, UStaticMeshDataAsset* _staticMesh, TArray<FVector> const& _locations)
    {
        TArray<FVector> locations = _locations;
        if (_mesh && _staticMesh)
        {
            _mesh->SetStaticMesh(_staticMesh->StaticMesh);
            _mesh->SetEnableGravity(false);
            addVoxel(_mesh, std::move(locations));
        }
    };

    lb_call(this->RedZoneMeshComponent, this->RedZoneDataAsset, _redZoneLocations);
    lb_call(this->WeaponMeshComponent, this->WeaponDataAsset, _attackLocations);
    lb_call(this->ProtectionMeshComponent, this->ProtectionDataAsset, _protectionLocations);
    lb_call(this->SupportMeshComponent, this->SupportDataAsset, _supportLocations);
}

void AShipPawn::RPCServerMove_Implementation(float const& _deltaTime)
{
    if (!ensure(this->DriverMeshComponent != nullptr)) return;
    if (!ensure(this->PlayerDataAsset != nullptr)) return;

    FVector angularVelocity = UKismetMathLibrary::NegateVector(this->DriverMeshComponent->GetPhysicsAngularVelocityInDegrees());
    angularVelocity *= 2.0f;

    this->DriverMeshComponent->AddTorqueInDegrees(angularVelocity, NAME_None, true);

    FVector const& linearVelocity = this->DriverMeshComponent->GetPhysicsLinearVelocity(NAME_None);
    FVector newVelocity = this->DriverMeshComponent->GetForwardVector() * this->PlayerDataAsset->MaxForwardSpeed * this->R_PercentSpeed;
    newVelocity = FMath::Lerp(linearVelocity, newVelocity, 0.9f);

    this->DriverMeshComponent->SetPhysicsLinearVelocity(newVelocity);

    RPCClientMove(angularVelocity, newVelocity);
}

void AShipPawn::RPCClientMove_Implementation(FVector const& _angularVelocity, FVector const& _linearVelocity)
{
    if (!ensure(this->DriverMeshComponent != nullptr)) return;
    this->DriverMeshComponent->AddTorqueInDegrees(_angularVelocity, NAME_None, true);
    this->DriverMeshComponent->SetPhysicsLinearVelocity(_linearVelocity);
}

void AShipPawn::fire(float const& _deltaTime)
{
    if (!ensure(this->PlayerDataAsset != nullptr)) return;
    if (!ensure(this->PlayerDataAsset->BulletClass != nullptr)) return;
    if (!ensure(this->WeaponMeshComponent != nullptr)) return;

    UWorld * world { this->GetWorld() };
    if (!ensure(world != nullptr)) return;
    
    // check if we have boolean for fire (only set on server)
    if (m_isFire.hasValue() && m_isFire.value() && m_fireCountDown <= 0.0f)
    {
        FTransform transform{};
        this->WeaponMeshComponent->GetInstanceTransform(m_fireIndex, transform, true);
        // reset scale
        transform.SetScale3D({1.0f, 1.0f, 1.0f});
        
        ++m_fireIndex;
        if (m_fireIndex >= this->WeaponMeshComponent->GetInstanceCount())
        {
            m_fireIndex = 0;
        }

        AActor* laser = Cast<AActor>(UGameplayStatics::BeginDeferredActorSpawnFromClass(world, this->PlayerDataAsset->BulletClass, transform));
        if (laser)
        {
            // init bullet
            laser->SetReplicates(true);
            laser->SetReplicateMovement(true);
            UGameplayStatics::FinishSpawningActor(laser, transform);
            if (UProjectileMovementComponent * comp = Cast<UProjectileMovementComponent>(laser->GetComponentByClass(UProjectileMovementComponent::StaticClass())))
            {
                comp->SetVelocityInLocalSpace(FVector(1, 0, 0) * comp->InitialSpeed);
            }
        }
    
        // reset count down
        m_fireCountDown = this->PlayerDataAsset->TimeBetweenFire;
    }
    else if (m_fireCountDown != 0.0f)
    {
        // we can't use timer manager here, because we want to keep timer when we release trigger
        // if player spam trigger and use timer manager, we will just spam the first tick of the handle timer
        // and throw many bullet
        m_fireCountDown -= _deltaTime;
    }
}

void AShipPawn::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(this->GetPlayerState());
    if (spacelPlayerState != nullptr)
    {
        FString teamName { spacelPlayerState->Team };
        if (teamName.Len() > 0)
        {
            // TO DO : Change color for teammate and ennemy team and our pawn
        }
    }
}

void AShipPawn::StartGame()
{
    ASpacelPlayerState* spacelPlayerState = this->GetPlayerState<ASpacelPlayerState>();
    if (spacelPlayerState == nullptr)
    {
        return;
    }

    TOptional<FTempArray> tmpArray { {} };
    buildRedZone(tmpArray);
    buildAttack(tmpArray, spacelPlayerState->Attack);
    buildProtection(tmpArray, spacelPlayerState->Protection);
    buildSupport(tmpArray, spacelPlayerState->Support);

    RPCClientAddVoxel(tmpArray.GetValue().RedZone, tmpArray.GetValue().Attack, tmpArray.GetValue().Protection, tmpArray.GetValue().Support);
}

void AShipPawn::buildRedZone(TOptional<FTempArray>& _tmpArray)
{
    if (!ensure(this->RedZoneMeshComponent != nullptr)) return;
    if (!ensure(this->RedZoneDataAsset != nullptr)) return;

    this->RedZoneMeshComponent->SetStaticMesh(this->RedZoneDataAsset->StaticMesh);
    this->RedZoneMeshComponent->SetEnableGravity(false);

    FVector location { 0, 0, 0 };
    FTransform voxelTransform{};
    voxelTransform.SetLocation(location);
    this->RedZoneMeshComponent->AddInstance(voxelTransform);

    if (_tmpArray.IsSet())
    {
        _tmpArray.GetValue().RedZone.Add(location);
    }
}

void AShipPawn::buildAttack(TOptional<FTempArray>& _tmpArray, uint8 _level)
{
    if (!ensure(this->WeaponMeshComponent != nullptr)) return;
    if (!ensure(this->WeaponDataAsset != nullptr)) return;

    FString const& path = _level > 0 ? this->WeaponDataAsset->HeavyPath : this->WeaponDataAsset->DefaultPath;

    this->WeaponMeshComponent->SetStaticMesh(this->WeaponDataAsset->StaticMesh);
    this->WeaponMeshComponent->SetEnableGravity(false);

    SimplyXml::FContainer<FVector> locationInformation { "Location" };
    SimplyXml::FContainer<FVector> fireInformation { "Fire" };
    SimplyXml::FReader reader { FPaths::ProjectDir() + path };
    reader.read(locationInformation, fireInformation);

    if (_tmpArray.IsSet())
    {
        _tmpArray.GetValue().Attack = locationInformation.Values;
    }

    // treatment of this information
    addVoxel(this->WeaponMeshComponent, std::move(locationInformation.Values));
}

void AShipPawn::buildProtection(TOptional<FTempArray>& _tmpArray, uint8 _level)
{
    if (!ensure(this->ProtectionMeshComponent != nullptr)) return;
    if (!ensure(this->ProtectionDataAsset != nullptr)) return;

    FString const& path = _level > 0 ? this->ProtectionDataAsset->HeavyPath : this->ProtectionDataAsset->DefaultPath;

    this->ProtectionMeshComponent->SetStaticMesh(this->ProtectionDataAsset->StaticMesh);
    this->ProtectionMeshComponent->SetEnableGravity(false);

    SimplyXml::FContainer<FVector> locationInformation{ "Location" };
    SimplyXml::FReader reader{ FPaths::ProjectDir() + path };
    reader.read(locationInformation);

    if (_tmpArray.IsSet())
    {
        _tmpArray.GetValue().Protection = locationInformation.Values;
    }

    // treatment of this information
    addVoxel(this->ProtectionMeshComponent, std::move(locationInformation.Values));
}

void AShipPawn::buildSupport(TOptional<FTempArray>& _tmpArray, uint8 _level)
{
    if (!ensure(this->SupportMeshComponent != nullptr)) return;
    if (!ensure(this->SupportDataAsset != nullptr)) return;

    FString const& path = _level > 0 ? this->SupportDataAsset->HeavyPath : this->SupportDataAsset->DefaultPath;

    this->SupportMeshComponent->SetStaticMesh(this->SupportDataAsset->StaticMesh);
    this->SupportMeshComponent->SetEnableGravity(false);

    SimplyXml::FContainer<FVector> locationInformation{ "Location" };
    SimplyXml::FReader reader{ FPaths::ProjectDir() + path };
    reader.read(locationInformation);

    if (_tmpArray.IsSet())
    {
        _tmpArray.GetValue().Support = locationInformation.Values;
    }

    // treatment of this information
    addVoxel(this->SupportMeshComponent, std::move(locationInformation.Values));
}

void AShipPawn::OnComponentHitProtection(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
    UE_LOG(LogTemp, Warning, TEXT("Protection"));
    return;
}

void AShipPawn::OnComponentHitRedZone(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
    UE_LOG(LogTemp, Warning, TEXT("Protection"));
    return;
}

void AShipPawn::OnComponentHitSupport(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
    UE_LOG(LogTemp, Warning, TEXT("Protection"));
    return;
}

void AShipPawn::BuildDefaultShip()
{
#if WITH_EDITOR
    // reset mesh
    if (this->RedZoneMeshComponent) this->RedZoneMeshComponent->ClearInstances();
    if (this->ProtectionMeshComponent) this->ProtectionMeshComponent->ClearInstances();
    if (this->WeaponMeshComponent) this->WeaponMeshComponent->ClearInstances();
    if (this->SupportMeshComponent) this->SupportMeshComponent->ClearInstances();
    TOptional<FTempArray> tmp {};
    buildRedZone(tmp);
    buildAttack(tmp, 0);
    buildProtection(tmp, 0);
    buildSupport(tmp, 0);
#endif
}

void AShipPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AShipPawn, R_PercentSpeed);
    DOREPLIFETIME(AShipPawn, RU_PercentFlightAttitude);
    DOREPLIFETIME(AShipPawn, RU_PercentTurn);
    DOREPLIFETIME(AShipPawn, RU_PercentUp);
}

