// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Source/DataAsset/ShipModuleDataAsset.h"
#include "Source/DataAsset/ProceduralModuleDataAsset.h"
#include "Source/Mesh/SpacelProceduralMeshComponent.h"
#include "Source/Player/SpacelPlayerState.h"
#include "Source/Enum/SpacelEnum.h"
#include "GameFramework/Controller.h"
#include "XmlFile.h"
#include "XmlNode.h"

// Sets default values
AShipCharacter::AShipCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    bAlwaysRelevant = true;

    ShipEngineComponent = CreateDefaultSubobject<USpacelProceduralMeshComponent>(TEXT("ShipEngine"));
    if (!ensure(ShipEngineComponent != nullptr)) return;
    ShipEngineComponent->bUseAsyncCooking = true;
    ShipEngineComponent->SetIsReplicated(true);
    ShipEngineComponent->SetupAttachment(RootComponent);

    ShipShellComponent = CreateDefaultSubobject<USpacelProceduralMeshComponent>(TEXT("ShipShell"));
    if (!ensure(ShipShellComponent != nullptr)) return;
    ShipShellComponent->bUseAsyncCooking = true;
    ShipShellComponent->SetIsReplicated(true);
    ShipShellComponent->SetupAttachment(RootComponent);

    // Create a spring arm component
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    if (!ensure(SpringArmComponent != nullptr)) return;
    SpringArmComponent->SetIsReplicated(true);
    SpringArmComponent->bUsePawnControlRotation = true;
    SpringArmComponent->SetupAttachment(RootComponent);	// Attach SpringArm to RootComponent

    // Create camera component 
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    if (!ensure(CameraComponent != nullptr)) return;
    CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName); // Attach the camera
    CameraComponent->bUsePawnControlRotation = false; // Don't rotate camera with controller
    CameraComponent->SetIsReplicated(true);
}

// Called when the game starts or when spawned
void AShipCharacter::BeginPlay()
{
	Super::BeginPlay();

    if (this->HasAuthority())
    {
        this->SetReplicates(true);
        this->SetReplicateMovement(true);
    }
}

// Called every frame
void AShipCharacter::Tick(float _deltaTime)
{
	Super::Tick(_deltaTime);

    if (!m_springArmDefaultSize.has_value())
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

    if (m_springArmDefaultSize.has_value())
    {
        // update spring arm size
        if (!ensure(this->SpringArmComponent != nullptr)) return;
        this->SpringArmComponent->TargetArmLength = m_springArmDefaultSize.value() + m_springArmDefaultSize.value() * this->PercentSpeed * this->MultiplierSpringArmSize;
    }
}

void AShipCharacter::RPCServerMove_Implementation(float const& _deltaTime)
{
    // rotation
    FRotator deltaRotation(0.f, 0.f, 0.f);
    if (this->PercentFlightAttitude != 0.0f)
    {
        deltaRotation.Roll += this->FlightAttitudeSpeed * this->PercentFlightAttitude * _deltaTime;
    }
    if (this->PercentTurn != 0.0f)
    {
        deltaRotation.Yaw += this->TurnSpeed * this->PercentTurn * _deltaTime;
    }
    if (this->PercentUp != 0.0f)
    {
        deltaRotation.Pitch += this->UpSpeed * this->PercentUp * _deltaTime;
    }

    this->AddControllerRollInput(deltaRotation.Roll);
    this->AddControllerYawInput(deltaRotation.Yaw);
    this->AddControllerPitchInput(deltaRotation.Pitch);

    // find out which way is forward
    FRotator rotation = this->Controller->GetControlRotation();
    rotation.Roll = 0;
    //get forward vector
    FVector direction = FRotationMatrix(rotation).GetUnitAxis(EAxis::X);
    direction  *= this->MaxForwardSpeed * this->PercentSpeed;
    this->AddMovementInput(direction);

    //FVector velocity = this->GetActorForwardVector() * this->MaxForwardSpeed * this->PercentSpeed;// * _deltaTime;
    //this->AddMovementInput(velocity);

    //RPCClientMove(velocity, deltaRotation);
    RPCClientMove(direction, deltaRotation);
}

void AShipCharacter::RPCClientMove_Implementation(FVector const& _velocity, FRotator const& _deltaRotation)
{
    this->AddControllerRollInput(_deltaRotation.Roll);
    this->AddControllerYawInput(_deltaRotation.Yaw);
    this->AddControllerPitchInput(_deltaRotation.Pitch);
    
    this->AddMovementInput(_velocity);

    //this->AddControllerRollInput(-10 * this->GetWorld()->DeltaTimeSeconds);
    //this->AddMovementInput(_velocity, 10);
}

void AShipCharacter::initShip()
{
    // save spring arm default size
    if (!ensure(this->SpringArmComponent != nullptr)) return;
    m_springArmDefaultSize = this->SpringArmComponent->TargetArmLength;

    // procedural mesh haven't built-in replication 
    BuildShip();
}

void AShipCharacter::BuildShip()
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

void AShipCharacter::buildProceduralModule(USpacelProceduralMeshComponent * _component, class UProceduralModuleDataAsset const* _module, FVector const& _location)
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
        _component->generateMesh(_module->Name);
        _component->SetMaterial(0, _module->Material);
    }
}

void AShipCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AShipCharacter, PercentSpeed);
    DOREPLIFETIME(AShipCharacter, PercentFlightAttitude);
    DOREPLIFETIME(AShipCharacter, PercentTurn);
    DOREPLIFETIME(AShipCharacter, PercentUp);
}