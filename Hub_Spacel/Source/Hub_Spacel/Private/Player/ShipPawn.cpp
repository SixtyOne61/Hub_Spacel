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
#include "Player/TargetActor.h"
#include "Player/FireComponent.h"
#include "GameState/SpacelGameState.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Hub_SpacelGameInstance.h"
#include "Util/SimplyXml.h"

// Sets default values
AShipPawn::AShipPawn()
{
    // Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
    bAlwaysRelevant = true;

    DriverMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Driver_00"));
    if (!ensure(DriverMeshComponent != nullptr)) return;
    DriverMeshComponent->SetCollisionProfileName("Player");
    RootComponent = DriverMeshComponent;

    BaseShipMeshComponent = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("ShipBase_00"));
    if (!ensure(BaseShipMeshComponent != nullptr)) return;
    BaseShipMeshComponent->SetupAttachment(RootComponent);

    ProtectionMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Protection_00"));
    if (!ensure(ProtectionMeshComponent != nullptr)) return;
    ProtectionMeshComponent->SetCollisionProfileName("Player");
    ProtectionMeshComponent->SetupAttachment(BaseShipMeshComponent);

    WeaponMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Weapon_00"));
    if (!ensure(WeaponMeshComponent != nullptr)) return;
    WeaponMeshComponent->SetCollisionProfileName("Player");
    WeaponMeshComponent->SetupAttachment(BaseShipMeshComponent);

    SupportMeshComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Support_00"));
    if (!ensure(SupportMeshComponent != nullptr)) return;
    SupportMeshComponent->SetCollisionProfileName("Player");
    SupportMeshComponent->SetupAttachment(BaseShipMeshComponent);

    // Create a spring arm component
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm_00"));
    if (!ensure(SpringArmComponent != nullptr)) return;
    SpringArmComponent->SetupAttachment(BaseShipMeshComponent);

    // Create camera component 
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera_00"));
    if (!ensure(CameraComponent != nullptr)) return;
    CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName); // Attach the camera

    FireComponent = CreateDefaultSubobject<UFireComponent>(TEXT("Fire_00"));

    TargetComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("Target_00"));
    if (!ensure(TargetComponent != nullptr)) return;
    TargetComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AShipPawn::BeginPlay()
{
	Super::BeginPlay();

    if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        ASpacelGameState* spacelGameState { Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())) };
        if (spacelGameState != nullptr)
        {
            spacelGameState->OnStartGameDelegate.AddDynamic(this, &AShipPawn::StartGame);
        }

        if (!ensure(this->DriverMeshComponent != nullptr)) return;
        this->DriverMeshComponent->OnComponentHit.AddDynamic(this, &AShipPawn::OnComponentHit);

        //if (!ensure(this->FireComponent != nullptr)) return;
        //this->FireComponent->Activate();
    }
    else if (!this->IsLocallyControlled())
    {
        if (!ensure(this->TargetComponent != nullptr)) return;
        TargetComponent->SetChildActorClass(this->TargetClass);
        TargetComponent->CreateChildActor();
    }
    else
    {
        UHub_SpacelGameInstance* spacelGameInstance{ Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
        spacelGameInstance->OnTargetPlayerDelegate.AddDynamic(this, &AShipPawn::OnTargetPlayer);
        spacelGameInstance->OnUnTargetDelegate.AddDynamic(this, &AShipPawn::OnUnTargetPlayer);
    }
}

void AShipPawn::OnTargetPlayer(AActor* _target)
{
    if (_target)
    {
        if (AShipPawn const* pawnOwner = Cast<AShipPawn>(_target->GetParentActor()))
        {
            if (ASpacelPlayerState* playerState = pawnOwner->GetPlayerState<ASpacelPlayerState>())
            {
                RPCServerTargetPlayer(playerState->PlayerId);
            }
        }
    }
}

void AShipPawn::RPCServerTargetPlayer_Implementation(int32 _playerId)
{
    if (!ensure(this->FireComponent != nullptr)) return;

    if (AGameStateBase* gameState = UGameplayStatics::GetGameState(this->GetWorld()))
    {
        for (APlayerState const* playerState : gameState->PlayerArray)
        {
            if (playerState)
            {
                if (playerState->PlayerId == _playerId)
                {
                    AActor* act = playerState->GetPawn();
                    this->FireComponent->m_target = act;
                    UE_LOG(LogTemp, Warning, TEXT("Target actor"));
                    break;
                }
            }
        }
    }
}

void AShipPawn::OnUnTargetPlayer()
{
    RPCServerUnTargetPlayer();
}

void AShipPawn::RPCServerUnTargetPlayer_Implementation()
{
    if (!ensure(this->FireComponent != nullptr)) return;
    this->FireComponent->m_target = nullptr;

    //UE_LOG(LogTemp, Warning, TEXT("UnTarget actor"));
}

// Called every frame
void AShipPawn::Tick(float _deltaTime)
{
	Super::Tick(_deltaTime);

    // collision ship
    handSweep();

    if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        // move ship
        RPCServerMove(_deltaTime);
    }
}

void AShipPawn::OnRep_PercentFlightAttitude()
{
    if (!ensure(this->DriverMeshComponent != nullptr)) return;
    if (!ensure(this->PlayerDataAsset != nullptr)) return;

    FVector dir { this->DriverMeshComponent->GetForwardVector() * this->RU_PercentFlightAttitude * this->PlayerDataAsset->FlightAttitudeSpeed };
    dir = FMath::Lerp(FVector::ZeroVector, dir, 0.1f);

    this->DriverMeshComponent->AddTorqueInDegrees(dir, NAME_None, true);
}

void AShipPawn::OnRep_PercentTurn()
{
    if (!ensure(this->DriverMeshComponent != nullptr)) return;
    if (!ensure(this->PlayerDataAsset != nullptr)) return;

    FVector dir { this->DriverMeshComponent->GetUpVector() * this->RU_PercentTurn * this->PlayerDataAsset->TurnSpeed };
    dir = FMath::Lerp(FVector::ZeroVector, dir, 0.1f);

    this->DriverMeshComponent->AddTorqueInDegrees(dir, NAME_None, true);
}

void AShipPawn::OnRep_PercentUp()
{
    if (!ensure(this->DriverMeshComponent != nullptr)) return;
    if (!ensure(this->PlayerDataAsset != nullptr)) return;

    FVector dir { this->DriverMeshComponent->GetRightVector() * this->RU_PercentUp * this->PlayerDataAsset->UpSpeed };
    dir = FMath::Lerp(FVector::ZeroVector, dir, 0.1f);

    this->DriverMeshComponent->AddTorqueInDegrees(dir, NAME_None, true);
}

void AShipPawn::RPCClientAddVoxel_Implementation(TArray<FVector> const& _redZoneLocations, TArray<FVector> const& _attackLocations, TArray<FVector> const& _protectionLocations, TArray<FVector> const& _supportLocations)
{
    buildShip(_redZoneLocations, _attackLocations, _protectionLocations, _supportLocations);
}

void AShipPawn::RPCServerMove_Implementation(float const& _deltaTime)
{
    if (!ensure(this->DriverMeshComponent != nullptr)) return;
    if (!ensure(this->PlayerDataAsset != nullptr)) return;

    FVector angularVelocity { UKismetMathLibrary::NegateVector(this->DriverMeshComponent->GetPhysicsAngularVelocityInDegrees()) };
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

void AShipPawn::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(this->GetPlayerState());
    if (spacelPlayerState != nullptr)
    {
        FString teamName{ spacelPlayerState->Team };
        if (teamName.Len() > 0)
        {
            // TO DO : Change color for teammate and ennemy team and our pawn
        }
    }
}

void AShipPawn::StartGame()
{
    FTempArray tmpArray {};
    tmpArray.RedZone.Add({ 0, 0, 0 });

    auto lb_readXml = [](uint8 _level, USetupAttributeDataAsset* _dataAsset, TArray<FVector> & _out)
    {
        if (!ensure(_dataAsset != nullptr)) return;
        FString const& path { _level > 0 ? _dataAsset->HeavyPath : _dataAsset->DefaultPath };

        SimplyXml::FContainer<FVector> locationInformation{ "Location" };
        SimplyXml::FReader reader{ FPaths::ProjectDir() + path };
        reader.read(locationInformation);

        _out = std::move(locationInformation.Values);
    };

    ASpacelPlayerState* spacelPlayerState = this->GetPlayerState<ASpacelPlayerState>();
    if (spacelPlayerState == nullptr)
    {
#if WITH_EDITOR
        lb_readXml(0, this->WeaponDataAsset, tmpArray.Attack);
        lb_readXml(0, this->ProtectionDataAsset, tmpArray.Protection);
        lb_readXml(0, this->SupportDataAsset, tmpArray.Support);

        buildShip(tmpArray.RedZone, tmpArray.Attack, tmpArray.Protection, tmpArray.Support);
#endif // WITH_EDITOR
    }
    else
    {
        lb_readXml(spacelPlayerState->Attack, this->WeaponDataAsset, tmpArray.Attack);
        lb_readXml(spacelPlayerState->Protection, this->ProtectionDataAsset, tmpArray.Protection);
        lb_readXml(spacelPlayerState->Support, this->SupportDataAsset, tmpArray.Support);

        buildShip(tmpArray.RedZone, tmpArray.Attack, tmpArray.Protection, tmpArray.Support);
        RPCClientAddVoxel(tmpArray.RedZone, tmpArray.Attack, tmpArray.Protection, tmpArray.Support);
    }
}

void AShipPawn::buildShip(TArray<FVector> const& _redZoneLocations, TArray<FVector> const& _attackLocations, TArray<FVector> const& _protectionLocations, TArray<FVector> const& _supportLocations)
{
    auto lb_call = [&](UInstancedStaticMeshComponent*& _mesh, UStaticMeshDataAsset* _staticMesh, TArray<FVector> const& _locations)
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
    };

    lb_call(this->WeaponMeshComponent, this->WeaponDataAsset, _attackLocations);
    lb_call(this->ProtectionMeshComponent, this->ProtectionDataAsset, _protectionLocations);
    lb_call(this->SupportMeshComponent, this->SupportDataAsset, _supportLocations);
}

void AShipPawn::BuildDefaultShip()
{
#if WITH_EDITOR
    StartGame();
#endif
}

void AShipPawn::setFire(bool _on)
{
    if (!ensure(this->FireComponent != nullptr)) return;
    this->FireComponent->m_isFire = _on;
}

void AShipPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AShipPawn, R_PercentSpeed);
    DOREPLIFETIME(AShipPawn, RU_PercentFlightAttitude);
    DOREPLIFETIME(AShipPawn, RU_PercentTurn);
    DOREPLIFETIME(AShipPawn, RU_PercentUp);
}

