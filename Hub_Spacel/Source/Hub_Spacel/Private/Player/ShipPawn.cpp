// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipPawn.h"
#include "Net/UnrealNetwork.h"
#include "XmlFile.h"
#include "XmlNode.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DataAsset/RedZoneDataAsset.h"
#include "DataAsset/ProtectionDataAsset.h"
#include "DataAsset/PlayerDataAsset.h"
#include "DataAsset/WeaponDataAsset.h"
#include "DataAsset/SupportDataAsset.h"
#include "Player/SpacelPlayerState.h"
#include "GameState/SpacelGameState.h"
#include "Components/InstancedStaticMeshComponent.h"

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
    RedZoneMeshComponent->SetupAttachment(BaseShipMeshComponent);

    ProtectionMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Protection_00"));
    if (!ensure(ProtectionMeshComponent != nullptr)) return;
    ProtectionMeshComponent->OnComponentHit.AddDynamic(this, &AShipPawn::OnComponentHitProtection);
    ProtectionMeshComponent->SetupAttachment(BaseShipMeshComponent);

    WeaponMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Weapon_00"));
    if (!ensure(WeaponMeshComponent != nullptr)) return;
    WeaponMeshComponent->SetupAttachment(BaseShipMeshComponent);

    SupportMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Support_00"));
    if (!ensure(SupportMeshComponent != nullptr)) return;
    SupportMeshComponent->OnComponentHit.AddDynamic(this, &AShipPawn::OnComponentHitSupport);
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

    ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld()));
    if (spacelGameState != nullptr)
    {
        spacelGameState->OnStartGameDelegate.AddDynamic(this, &AShipPawn::StartGame);
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

void AShipPawn::addVoxelFromXml(UInstancedStaticMeshComponent* _mesh, FString const& _filePath)
{
    if (!ensure(_mesh != nullptr)) return;

    FXmlFile file;
    if (!file.LoadFile(_filePath))
    {
        return;
    }

    FXmlNode* rootNode { file.GetRootNode() };
    if (rootNode == nullptr)
    {
        return;
    }

    TArray<FXmlNode*> const& childrenNodes { rootNode->GetChildrenNodes() };
    for (auto const* node : childrenNodes)
    {
        if (node != nullptr && node->GetTag() == "Location")
        {
            FVector location {};
            location.InitFromString(node->GetAttribute("val"));
            FTransform voxelTransform{};
            voxelTransform.SetLocation(location);
            _mesh->AddInstance(voxelTransform);
        }
    }
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
        FVector location {};
        m_fireLocations.Dequeue(location);
        FTransform transform {};
        transform.SetLocation(this->WeaponMeshComponent->GetRelativeLocation() + location + this->GetActorLocation());
        transform.SetRotation(this->GetActorRotation().Quaternion());
    
        // re-add to the end this fire locations
        m_fireLocations.Enqueue(location);

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

    buildRedZone();
    buildAttack(spacelPlayerState->getSkillPoint(ESkillType::Attack));
    buildProtection(spacelPlayerState->getSkillPoint(ESkillType::Protection));
    buildSupport(spacelPlayerState->getSkillPoint(ESkillType::Support));
}

void AShipPawn::buildRedZone()
{
    if (!ensure(this->RedZoneMeshComponent != nullptr)) return;
    if (!ensure(this->RedZoneDataAsset != nullptr)) return;

    this->RedZoneMeshComponent->SetStaticMesh(this->RedZoneDataAsset->RedZoneStaticMesh);
    this->RedZoneMeshComponent->SetEnableGravity(false);

    FVector location { 0, 0, 0};
    FTransform voxelTransform{};
    voxelTransform.SetLocation(location);
    this->RedZoneMeshComponent->AddInstance(voxelTransform);
}

void AShipPawn::buildAttack(uint8 _level)
{
    if (!ensure(this->WeaponMeshComponent != nullptr)) return;
    if (!ensure(this->WeaponDataAsset != nullptr)) return;

    UStaticMesh* mesh = this->WeaponDataAsset->WeaponMesh;
    FString const& path = _level > 0 ? this->WeaponDataAsset->HeavyWeaponPath : this->WeaponDataAsset->DefaultWeaponPath;

    this->WeaponMeshComponent->SetStaticMesh(mesh);
    this->WeaponMeshComponent->SetEnableGravity(false);
    addVoxelFromXml(this->WeaponMeshComponent, FPaths::ProjectDir() + path);

    // TO DO : read shoot point from xml
    FXmlFile file;
    if (!file.LoadFile(FPaths::ProjectDir() + path)) return;

    FXmlNode* rootNode{ file.GetRootNode() };
    if (rootNode == nullptr) return;

    TArray<FXmlNode*> const& childrenNodes{ rootNode->GetChildrenNodes() };
    for (auto const* node : childrenNodes)
    {
        if (node != nullptr && node->GetTag() == "Fire")
        {
            FVector location{};
            location.InitFromString(node->GetAttribute("val"));
            m_fireLocations.Enqueue(location);
        }
    }
}

void AShipPawn::buildProtection(uint8 _level)
{
    if (!ensure(this->ProtectionMeshComponent != nullptr)) return;
    if (!ensure(this->ProtectionDataAsset != nullptr)) return;

    UStaticMesh* mesh = _level > 0 ? this->ProtectionDataAsset->HeavyStaticMesh : this->ProtectionDataAsset->DefaultStaticMesh;
    FString const& path = _level > 0 ? this->ProtectionDataAsset->HeavyProtectionPath : this->ProtectionDataAsset->DefaultProtectionPath;

    this->ProtectionMeshComponent->SetStaticMesh(mesh);
    this->ProtectionMeshComponent->SetEnableGravity(false);
    addVoxelFromXml(this->ProtectionMeshComponent, FPaths::ProjectDir() + path);
}

void AShipPawn::buildSupport(uint8 _level)
{
    if (!ensure(this->SupportMeshComponent != nullptr)) return;
    if (!ensure(this->SupportDataAsset != nullptr)) return;

    UStaticMesh* mesh = this->SupportDataAsset->SupportMesh;
    FString const& path = _level > 0 ? this->SupportDataAsset->HeavySupportPath : this->SupportDataAsset->DefaultSupportPath;

    this->SupportMeshComponent->SetStaticMesh(mesh);
    this->SupportMeshComponent->SetEnableGravity(false);
    addVoxelFromXml(this->SupportMeshComponent, FPaths::ProjectDir() + path);
}

void AShipPawn::OnComponentHitProtection(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
    return;
}

void AShipPawn::OnComponentHitRedZone(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
    return;
}

void AShipPawn::OnComponentHitSupport(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit)
{
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
    buildRedZone();
    buildAttack(0);
    buildProtection(0);
    buildSupport(0);
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

