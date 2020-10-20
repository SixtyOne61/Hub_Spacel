// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipPawn.h"
#include "ShipPawnMovement.h"
#include "Materials/MaterialInstance.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Source/Mesh/SpacelProceduralMeshComponent.h"
#include "Source/Player/SpacelPlayerState.h"
#include "XmlFile.h"
#include "XmlNode.h"
#include "Camera/CameraComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Source/DataAsset/ShipModuleDataAsset.h"
#include "Source/DataAsset/ProceduralModuleDataAsset.h"
#include "Source/Enum/SpacelEnum.h"

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

    // Create a spring arm component
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm_00"));
    if (!ensure(SpringArmComponent != nullptr)) return;
    SpringArmComponent->SetupAttachment(BaseShipMeshComponent);

    // Create camera component 
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera_00"));
    if (!ensure(CameraComponent != nullptr)) return;
    CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName); // Attach the camera

    /*ShipBaseComponent = CreateDefaultSubobject<USpacelProceduralMeshComponent>(TEXT("ShipBase_00"));
    if (!ensure(ShipBaseComponent != nullptr)) return;
    ShipBaseComponent->bUseAsyncCooking = true;
    ShipBaseComponent->SetIsReplicated(true);
    RootComponent = ShipBaseComponent;

    ShipPawnMovement = CreateDefaultSubobject<UShipPawnMovement>(TEXT("ShipPawnMovement_00"));
    if (!ensure(ShipPawnMovement != nullptr)) return;
    ShipPawnMovement->SetIsReplicated(true);
    this->ShipPawnMovement->UpdatedComponent = RootComponent;*/
}

// Called when the game starts or when spawned
void AShipPawn::BeginPlay()
{
	Super::BeginPlay();

    /*if (this->HasAuthority())
    {
        this->SetReplicates(true);
        this->SetReplicateMovement(true);
    }

    this->ShipPawnMovement->SetComponentTickEnabled(true);*/
}

// Called every frame
void AShipPawn::Tick(float _deltaTime)
{
	Super::Tick(_deltaTime);

    if (this->HasAuthority())
    {
        RPCServerMove(_deltaTime);
    }

    /*if (!m_springArmDefaultSize.has_value())
    {
        // wait player state before start building
        if (this->GetPlayerState() != nullptr)
        {
            initShip();
        }
    }

    if (this->HasAuthority())
    {
        RPCServerMove(_deltaTime);
    }

    if(m_springArmDefaultSize.has_value())
    {
        // update spring arm size
        if (!ensure(this->SpringArmComponent != nullptr)) return;
        this->SpringArmComponent->TargetArmLength = m_springArmDefaultSize.value() + m_springArmDefaultSize.value() * this->PercentSpeed * this->MultiplierSpringArmSize;
    }*/
}

void AShipPawn::OnRep_PercentSpeed()
{

}

void AShipPawn::OnRep_PercentFlightAttitude()
{
    if (!ensure(this->DriverMeshComponent != nullptr)) return;

    FVector dir = this->DriverMeshComponent->GetForwardVector() * this->PercentFlightAttitude * this->FlightAttitudeSpeed;
    dir = FMath::Lerp(FVector::ZeroVector, dir, 0.1f);

    this->DriverMeshComponent->AddTorqueInDegrees(dir, NAME_None, true);
}

void AShipPawn::OnRep_PercentTurn()
{
    if (!ensure(this->DriverMeshComponent != nullptr)) return;

    FVector dir = this->DriverMeshComponent->GetUpVector() * this->PercentTurn * this->TurnSpeed;
    dir = FMath::Lerp(FVector::ZeroVector, dir, 0.1f);

    this->DriverMeshComponent->AddTorqueInDegrees(dir, NAME_None, true);
}

void AShipPawn::OnRep_PercentUp()
{
    if (!ensure(this->DriverMeshComponent != nullptr)) return;

    FVector dir = this->DriverMeshComponent->GetRightVector() * this->PercentUp * this->UpSpeed;
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

        TArray<TSharedPtr<ChainedLocation>> chainedLocations;
        chainedLocations.Reserve(childrenNodes.Num() - 1);

        FVector loc;
        unsigned int nbNode = (unsigned int)childrenNodes.Num();
        for (unsigned int i = 1; i < nbNode; ++i)
        {
            if (FXmlNode const* node = childrenNodes[i])
            {
                loc.InitFromString(node->GetAttribute("val"));
                chainedLocations.Add(MakeShareable(new ChainedLocation(loc, cubeSize, -1)));
            }
        }

        _component->setEdges(std::forward<TArray<TSharedPtr<ChainedLocation>>>(chainedLocations));
        _component->generateMesh("NoCollision");
        _component->SetMaterial(0, _module->Material);
    }
}

void AShipPawn::RPCServerMove_Implementation(float const& _deltaTime)
{
    if (!ensure(this->DriverMeshComponent != nullptr)) return;

    FVector angularVelocity = UKismetMathLibrary::NegateVector(this->DriverMeshComponent->GetPhysicsAngularVelocityInDegrees());
    angularVelocity *= 2.0f;

    this->DriverMeshComponent->AddTorqueInDegrees(angularVelocity, NAME_None, true);

    FVector const& linearVelocity = this->DriverMeshComponent->GetPhysicsLinearVelocity(NAME_None);
    FVector newVelocity = this->DriverMeshComponent->GetForwardVector() * this->MaxForwardSpeed * this->PercentSpeed;
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

void AShipPawn::initShip()
{
    // save spring arm default size
    //if (!ensure(this->SpringArmComponent != nullptr)) return;
    //m_springArmDefaultSize = this->SpringArmComponent->TargetArmLength;

    // procedural mesh haven't built-in replication 
    //BuildShip();
}

void AShipPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AShipPawn, PercentSpeed);
    DOREPLIFETIME(AShipPawn, PercentFlightAttitude);
    DOREPLIFETIME(AShipPawn, PercentTurn);
    DOREPLIFETIME(AShipPawn, PercentUp);
}

