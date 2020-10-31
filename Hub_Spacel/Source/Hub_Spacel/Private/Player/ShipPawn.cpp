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
#include "DataAsset/ShipModuleDataAsset.h"
#include "DataAsset/ProceduralModuleDataAsset.h"
#include "DataAsset/PlayerDataAsset.h"
#include "Mesh/SpacelProceduralMeshComponent.h"
#include "Player/SpacelPlayerState.h"
#include "Enum/SpacelEnum.h"
#include "Mesh/LocationInformation.h"

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

    ShipEngineComponent = CreateDefaultSubobject<USpacelProceduralMeshComponent>(TEXT("ShipEngine_00"));
    if (!ensure(ShipEngineComponent != nullptr)) return;
    ShipEngineComponent->bUseAsyncCooking = true;
    ShipEngineComponent->SetupAttachment(BaseShipMeshComponent);

    ShipShellComponent = CreateDefaultSubobject<USpacelProceduralMeshComponent>(TEXT("ShipShell_00"));
    if (!ensure(ShipShellComponent != nullptr)) return;
    ShipShellComponent->bUseAsyncCooking = true;
    ShipShellComponent->SetupAttachment(BaseShipMeshComponent);

    SubMachineComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SubMachine_00"));
    if (!ensure(SubMachineComponent != nullptr)) return;
    SubMachineComponent->SetupAttachment(BaseShipMeshComponent);

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

    FVector dir = this->DriverMeshComponent->GetForwardVector() * this->PercentFlightAttitude * this->PlayerDataAsset->FlightAttitudeSpeed;
    dir = FMath::Lerp(FVector::ZeroVector, dir, 0.1f);

    this->DriverMeshComponent->AddTorqueInDegrees(dir, NAME_None, true);
}

void AShipPawn::OnRep_PercentTurn()
{
    if (!ensure(this->DriverMeshComponent != nullptr)) return;
    if (!ensure(this->PlayerDataAsset != nullptr)) return;

    FVector dir = this->DriverMeshComponent->GetUpVector() * this->PercentTurn * this->PlayerDataAsset->TurnSpeed;
    dir = FMath::Lerp(FVector::ZeroVector, dir, 0.1f);

    this->DriverMeshComponent->AddTorqueInDegrees(dir, NAME_None, true);
}

void AShipPawn::OnRep_PercentUp()
{
    if (!ensure(this->DriverMeshComponent != nullptr)) return;
    if (!ensure(this->PlayerDataAsset != nullptr)) return;

    FVector dir = this->DriverMeshComponent->GetRightVector() * this->PercentUp * this->PlayerDataAsset->UpSpeed;
    dir = FMath::Lerp(FVector::ZeroVector, dir, 0.1f);

    this->DriverMeshComponent->AddTorqueInDegrees(dir, NAME_None, true);
}

void AShipPawn::BuildShip()
{
    if (!ensure(this->ModuleDataAsset != nullptr)) return;

    FVector const& location = this->GetActorLocation();
    ASpacelPlayerState * spacelPlayerState = this->GetPlayerState<ASpacelPlayerState>();
    if (spacelPlayerState)
    {
        buildProceduralModule(this->ShipEngineComponent, this->ModuleDataAsset->GetModule(spacelPlayerState->ShipEngineModuleType), location);
        buildProceduralModule(this->ShipShellComponent, this->ModuleDataAsset->GetModule(spacelPlayerState->ShipShellModuleType), location);
    }
    else
    {
#ifdef  WITH_EDITOR
        // only for editor mode
        buildProceduralModule(this->ShipEngineComponent, this->ModuleDataAsset->GetModule((uint8)EShipModuleType::EngineDefault), location);
        buildProceduralModule(this->ShipShellComponent, this->ModuleDataAsset->GetModule((uint8)EShipModuleType::ShellDefault), location);
#endif //  WITH_EDITOR
    }
}

void AShipPawn::buildProceduralModule(USpacelProceduralMeshComponent * _component, class UProceduralModuleDataAsset const* _module, FVector const& _location)
{
    if (!ensure(_component != nullptr)) return;
    if (!ensure(_module != nullptr)) return;

    FString path = FPaths::ProjectDir() + _module->Path;
    FXmlFile file;
    if (!file.LoadFile(path))
    {
        return;
    }

    FXmlNode * rootNode = file.GetRootNode();
    if (rootNode == nullptr)
    {
        return;
    }

    FXmlNode const* modNode = rootNode->GetFirstChildNode();
    if (modNode == nullptr)
    {
        return;
    }

    TArray<FXmlNode*> const& childrenNodes = modNode->GetChildrenNodes();
    if (childrenNodes.Num() && childrenNodes[0] && childrenNodes[0]->GetTag() == "Size")
    {
        FVector cubeSize;
        cubeSize.InitFromString(childrenNodes[0]->GetAttribute("val"));

        _component->SetWorldLocation(_location);
        _component->setCubeSize(cubeSize);

        TArray<FLocationInformation> locations;
        int size = childrenNodes.Num();
        locations.Reserve(size);
        locations.SetNum(size);

        FVector loc;
        unsigned int nbNode = (unsigned int)childrenNodes.Num();
        for (unsigned int i = 1; i < nbNode; ++i)
        {
            if (FXmlNode const* node = childrenNodes[i])
            {
                loc.InitFromString(node->GetAttribute("val"));
                locations[i].Location = loc;
                locations[i].Used = true;
            }
        }

        _component->setEdges(std::move(locations));
        _component->generateMesh("NoCollision", locations.Num());
        _component->SetMaterial(0, _module->Material);
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
    FVector newVelocity = this->DriverMeshComponent->GetForwardVector() * this->PlayerDataAsset->MaxForwardSpeed * this->PercentSpeed;
    newVelocity = FMath::Lerp(linearVelocity, newVelocity, 0.01f);

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
    if (!ensure(this->SubMachineComponent != nullptr)) return;
    UWorld * world = this->GetWorld();
    if (!ensure(world != nullptr)) return;

    // check if we have boolean for fire (only set on server)
    if (m_isFire.hasValue() && m_isFire.value() && m_fireCountDown <= 0.0f)
    {
        FVector location = this->SubMachineComponent->GetRelativeLocation() + this->GetActorLocation();
        FTransform transform {};
        transform.SetLocation(location);
        transform.SetRotation(this->GetActorRotation().Quaternion());

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

void AShipPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AShipPawn, PercentSpeed);
    DOREPLIFETIME(AShipPawn, PercentFlightAttitude);
    DOREPLIFETIME(AShipPawn, PercentTurn);
    DOREPLIFETIME(AShipPawn, PercentUp);
}

