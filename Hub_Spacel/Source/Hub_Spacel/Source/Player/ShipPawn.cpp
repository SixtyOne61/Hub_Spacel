// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipPawn.h"
#include "ShipPawnMovement.h"
#include "Materials/MaterialInstance.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"
#include "Source/Mesh/SpacelProceduralMeshComponent.h"
#include "Source/Player/SpacelPlayerState.h"
#include "Source/DataAsset/ShipModuleDataAsset.h"
#include "Source/DataAsset/ProceduralModuleDataAsset.h"

// Sets default values
AShipPawn::AShipPawn()
{
    // Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    ShipBaseComponent = CreateDefaultSubobject<USpacelProceduralMeshComponent>(TEXT("ShipBase"));
    if (!ensure(ShipBaseComponent != nullptr)) return;
    ShipBaseComponent->bUseAsyncCooking = true;
    ShipBaseComponent->SetIsReplicated(true);
    RootComponent = ShipBaseComponent;

    ShipEngineComponent = CreateDefaultSubobject<USpacelProceduralMeshComponent>(TEXT("ShipEngine"));
    if (!ensure(ShipEngineComponent != nullptr)) return;
    ShipEngineComponent->bUseAsyncCooking = true;
    ShipEngineComponent->SetIsReplicated(true);
    ShipEngineComponent->SetupAttachment(ShipBaseComponent);

    ShipPawnMovement = CreateDefaultSubobject<UShipPawnMovement>(TEXT("ShipPawnMovement"));
    if (!ensure(ShipPawnMovement != nullptr)) return;
    ShipPawnMovement->SetIsReplicated(true);

    // Create a spring arm component
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    if (!ensure(SpringArmComponent != nullptr)) return;
    SpringArmComponent->SetIsReplicated(true);
    SpringArmComponent->SetupAttachment(ShipBaseComponent);	// Attach SpringArm to RootComponent

    // Create camera component 
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    if (!ensure(CameraComponent != nullptr)) return;
    CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName); // Attach the camera
    CameraComponent->bUsePawnControlRotation = false; // Don't rotate camera with controller
    CameraComponent->SetIsReplicated(true);
}

// Called when the game starts or when spawned
void AShipPawn::BeginPlay()
{
	Super::BeginPlay();

    if (this->HasAuthority())
    {
        this->SetReplicates(true);
        this->SetReplicateMovement(true);
    }
}

// Called every frame
void AShipPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (!m_isBuild)
    {
        // wait player state before start building
        if (this->GetPlayerState() != nullptr)
        {
            initShip();
            m_isBuild = true;
        }
    }

    move();

    if(m_isBuild)
    {
        // update spring arm size
        if (!ensure(this->SpringArmComponent != nullptr)) return;
        this->SpringArmComponent->TargetArmLength = m_springArmDefaultSize + m_springArmDefaultSize * this->PercentSpeed * this->MultiplierSpringArmSize;
    }
}

void AShipPawn::NotifyHit(class UPrimitiveComponent* _myComp, class AActor* _other, class UPrimitiveComponent* _otherComp, bool _bSelfMoved, FVector _hitLocation, FVector _hitNormal, FVector _normalImpulse, const FHitResult& _hit)
{

}

void AShipPawn::buildShip()
{
    if (!ensure(this->ModuleDataAsset != nullptr)) return;

    ASpacelPlayerState * spacelPlayerState = this->GetPlayerState<ASpacelPlayerState>();
    if (!ensure(spacelPlayerState != nullptr)) return;

    FVector const& location = this->GetActorLocation();
    buildProceduralModule(this->ShipBaseComponent, this->ModuleDataAsset->GetModule(spacelPlayerState->ShipBaseModuleType), location);
    buildProceduralModule(this->ShipEngineComponent, this->ModuleDataAsset->GetModule(spacelPlayerState->ShipEngineModuleType), location);
}

void AShipPawn::buildProceduralModule(USpacelProceduralMeshComponent * _component, class UProceduralModuleDataAsset const* _module, FVector const& _location)
{
    if (!ensure(_component != nullptr)) return;
    if (!ensure(_module != nullptr)) return;

    _component->SetWorldLocation(_location);
    FVector const& cubeSize = _module->CubeSize;
    _component->setCubeSize(cubeSize);
    TArray<TSharedPtr<ChainedLocation>> chainedLocations;
    chainedLocations.Reserve(_module->MeshSetup.Num());
    for (auto const& loc : _module->MeshSetup)
    {
        chainedLocations.Add(MakeShareable(new ChainedLocation(loc, cubeSize)));
    }

    _component->setEdges(std::forward<TArray<TSharedPtr<ChainedLocation>>>(chainedLocations));
    _component->generateMesh(_module->Name);
    _component->SetMaterial(0, _module->Material);
}

void AShipPawn::move()
{
    if(!ensure(this->ShipPawnMovement != nullptr)) return;
    this->ShipPawnMovement->AddInputVector(this->GetActorForwardVector() * this->MaxForwardSpeed * this->PercentSpeed);
}

void AShipPawn::initShip()
{
    // save spring arm default size
    if (!ensure(this->SpringArmComponent != nullptr)) return;
    m_springArmDefaultSize = this->SpringArmComponent->TargetArmLength;

    // procedural mesh haven't built-in replication 
    buildShip();
}

void AShipPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AShipPawn, PercentSpeed);
}

