// Fill out your copyright notice in the Description page of Project Settings.


#include "Hub_Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "Source/Mesh/SpacelProceduralMeshComponent.h"
#include "Materials/MaterialInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"
#include <functional>

// Sets default values
AHub_Pawn::AHub_Pawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    // create procedural mesh component
    ProceduralSpaceShipBase = CreateDefaultSubobject<USpacelProceduralMeshComponent>(TEXT("ProceduralBase0"));
    ProceduralSpaceShipBase->bUseAsyncCooking = true;
    RootComponent = ProceduralSpaceShipBase;

    ProceduralSpaceShipShell = CreateDefaultSubobject<USpacelProceduralMeshComponent>(TEXT("ProceduralShell0"));
    ProceduralSpaceShipShell->bUseAsyncCooking = true;
    ProceduralSpaceShipShell->SetupAttachment(RootComponent);

    ProceduralSpaceShipEngine = CreateDefaultSubobject<USpacelProceduralMeshComponent>(TEXT("ProceduralEngine0"));
    ProceduralSpaceShipEngine->bUseAsyncCooking = true;
    ProceduralSpaceShipEngine->SetupAttachment(RootComponent);

	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->SetupAttachment(ProceduralSpaceShipShell);	// Attach SpringArm to RootComponent
	SpringArm->TargetArmLength = 260.0f; // The camera follows at this distance behind the character	
	SpringArm->SocketOffset = FVector(0.f, 0.f, 90.f);
	SpringArm->bEnableCameraLag = true;	// Do not allow camera to lag
	SpringArm->CameraLagSpeed = 600.f;

	// Create camera component 
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);	// Attach the camera
	Camera->bUsePawnControlRotation = false; // Don't rotate camera with controller
}

// Called when the game starts or when spawned
void AHub_Pawn::BeginPlay()
{
	Super::BeginPlay();
	
    generateMesh();
    resetCrosshair();

    // init default rotation for snap
    if (this->ProceduralSpaceShipShell)
    {
        this->m_defaultRotation = this->ProceduralSpaceShipShell->GetRelativeTransform().Rotator();
    }
}

// Called every frame
void AHub_Pawn::Tick(float _deltaTime)
{
	// move
	const FVector localMove = FVector(this->m_currentForwardSpeed * _deltaTime, 0.f, 0.f);

	// Move plan forwards (with sweep so we stop when we collide with things)
	AddActorLocalOffset(localMove, true);

	// rotation
	FRotator deltaRotation(0.f, 0.f, 0.f);
	deltaRotation.Pitch = m_currentPitchSpeed * _deltaTime;
	deltaRotation.Yaw = m_currentYawSpeed * _deltaTime;
	deltaRotation.Roll = m_currentRollSpeed * _deltaTime;

	// rotate ship
	AddActorLocalRotation(deltaRotation);

    snapTarget(_deltaTime);
    fireLaser(_deltaTime);

	// Call any parent class Tick implementation
	Super::Tick(_deltaTime);
}

void AHub_Pawn::NotifyHit(class UPrimitiveComponent* _myComp, class AActor* _other, class UPrimitiveComponent* _otherComp, bool _bSelfMoved, FVector _hitLocation, FVector _hitNormal, FVector _normalImpulse, const FHitResult& _hit)
{
	Super::NotifyHit(_myComp, _other, _otherComp, _bSelfMoved, _hitLocation, _hitNormal, _normalImpulse, _hit);

	// Deflect along the surface when we collide.
	FRotator currentRotation = GetActorRotation();
	SetActorRotation(FQuat::Slerp(currentRotation.Quaternion(), _hitNormal.ToOrientationQuat(), 0.025f));
}

// Called to bind functionality to input
void AHub_Pawn::SetupPlayerInputComponent(UInputComponent* _playerInputComponent)
{
	Super::SetupPlayerInputComponent(_playerInputComponent);

	// Check if PlayerInputComponent is valid (not NULL)
	check(_playerInputComponent);
	
    /* bind input rework */
    _playerInputComponent->BindAxis("MoveUp", this, &AHub_Pawn::input_MoveUp);
    _playerInputComponent->BindAxis("MoveRight", this, &AHub_Pawn::input_MoveRight);
    _playerInputComponent->BindAxis("MoveTargetUp", this, &AHub_Pawn::input_MoveTargetUp);
    _playerInputComponent->BindAxis("MoveTargetRight", this, &AHub_Pawn::input_MoveTargetRight);
    _playerInputComponent->BindAxis("Speed", this, &AHub_Pawn::input_Speed);

    _playerInputComponent->BindAction("Snap", IE_Pressed, this, &AHub_Pawn::input_SnapOn);
    _playerInputComponent->BindAction("Snap", IE_Released, this, &AHub_Pawn::input_SnapOff);
    _playerInputComponent->BindAction("Fire", IE_Pressed, this, &AHub_Pawn::input_FireOn);
    _playerInputComponent->BindAction("Fire", IE_Released, this, &AHub_Pawn::input_FireOff);
}

void AHub_Pawn::input_FireOn()
{
    this->m_isFire = true;
}

void AHub_Pawn::input_FireOff()
{
    this->m_isFire = false;
}

void AHub_Pawn::input_Speed(float _val)
{
    // TO DO : not cool + _val
    this->PercentSpeed = FMath::Clamp(this->PercentSpeed + _val, 0.0f, 100.0f);
    this->m_currentForwardSpeed = this->MaxForwardSpeed * (this->PercentSpeed / 100.0f);
}

void AHub_Pawn::input_MoveUp(float _val)
{
	// target pitch speed is based in input
	float targetPitchSpeed = (_val * this->TurnSpeed * -1.0f);

	// when steering, we decrease pitch slightly
	targetPitchSpeed += (FMath::Abs(this->m_currentYawSpeed) * -0.2f);

	// Smoothly interpolate to target pitch speed
    this->m_currentPitchSpeed = FMath::FInterpTo(this->m_currentPitchSpeed, targetPitchSpeed, GetWorld()->GetDeltaSeconds(), this->InterpSpeed);
}

void AHub_Pawn::input_MoveRight(float _val)
{
	// target yaw speed is based on input
	float targetYawSpeed = (_val * this->TurnSpeed);

	// smoothly interpolate to target yaw speed
    this->m_currentYawSpeed = FMath::FInterpTo(this->m_currentYawSpeed, targetYawSpeed, GetWorld()->GetDeltaSeconds(), this->InterpSpeed);

	// Is there any left / right input ?
	const bool isTurning = FMath::Abs(_val) > 0.2f;

	// if turning, yaw value is used to influence rool
	// if not turning, roll to reverse current roll value
	float targetRollSpeed = isTurning ? (this->m_currentYawSpeed * 0.8f) : (GetActorRotation().Roll * -2.0f);

	// smoothly interpolate roll speed
    this->m_currentRollSpeed = FMath::FInterpTo(this->m_currentRollSpeed, targetRollSpeed, GetWorld()->GetDeltaSeconds(), this->InterpSpeed);
}

void AHub_Pawn::input_MoveTargetUp(float _val)
{
    if (!m_isSnap)
    {
        return;
    }

    float sensibility = this->SensibilityCrosshair;
    float delta = this->m_viewportSize.X * _val * sensibility;
    this->CrosshairPosition.X = FMath::Clamp(this->CrosshairPosition.X + delta, 0.0f, this->m_viewportSize.X);
}

void AHub_Pawn::input_MoveTargetRight(float _val)
{
    if (!this->m_isSnap)
    {
        return;
    }

    float sensibility = this->SensibilityCrosshair;
    float delta = this->m_viewportSize.Y * _val * sensibility;
    this->CrosshairPosition.Y = FMath::Clamp(this->CrosshairPosition.Y + delta, 0.0f, this->m_viewportSize.Y);
}

void AHub_Pawn::input_SnapOn()
{
    this->m_isSnap = true;
}

void AHub_Pawn::input_SnapOff()
{
    this->m_isSnap = false;
    this->m_progressResetSnap = this->TimeToResetSnap;
    if (this->ProceduralSpaceShipShell)
    {
        this->m_snapRotationOnRelease = this->ProceduralSpaceShipShell->GetComponentRotation();
        this->m_snapRelativeRotationOnRelease = this->ProceduralSpaceShipShell->GetRelativeTransform().Rotator();
    }

    resetCrosshair();
}

void AHub_Pawn::fireLaser(float _deltaTime)
{
    if (this->m_isFire && this->m_laserCountDown <= 0.0f)
    {
        FTransform transform = ProceduralSpaceShipBase->GetSocketTransform("SimpleBulletSpawn");
        AActor* pLaser = Cast<AActor>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), this->LaserClass, transform));
        if (pLaser)
        {
            // TO DO init bullet
            pLaser->SetReplicates(true);
            UGameplayStatics::FinishSpawningActor(pLaser, transform);
            UProjectileMovementComponent* comp = Cast<UProjectileMovementComponent>(pLaser->GetComponentByClass(UProjectileMovementComponent::StaticClass()));
            if (comp && this->ProceduralSpaceShipShell)
            {
                comp->SetVelocityInLocalSpace(FVector(1, 0, 0) * comp->InitialSpeed);
            }
        }
        this->m_laserCountDown = this->TimeBetweenLaserShot;
    }
    else if (this->m_laserCountDown != 0.0f)
    {
        this->m_laserCountDown -= _deltaTime;
    }
}

void AHub_Pawn::generateMesh()
{
    FVector const& location = GetActorLocation();

    auto lb_init = [&location](USpacelProceduralMeshComponent * _mesh, std::function<void(void)> _func)
    {
        if (!_mesh)
        {
            return;
        }
        _mesh->SetWorldLocation(location);
        _func();
    };

    lb_init(this->ProceduralSpaceShipBase, std::bind(&AHub_Pawn::generateBase, this));
    lb_init(this->ProceduralSpaceShipShell, std::bind(&AHub_Pawn::generateShell, this));
    lb_init(this->ProceduralSpaceShipEngine, std::bind(&AHub_Pawn::generateEngine, this));
}

void AHub_Pawn::generateBase()
{
    FVector cubeSize = FVector(15.0f, 15.0f, 15.0f);
    ProceduralSpaceShipBase->setCubeSize(cubeSize);
    TArray<TSharedPtr<ChainedLocation>> chainedLocations =
    {
        MakeShareable(new ChainedLocation(FVector(-15,0,0), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(0,0,0), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(15,0,0), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(0,15,0), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(0,-15,0), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(0,0,15), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(0,0,-15), cubeSize)),
    };
    this->ProceduralSpaceShipBase->setEdges(std::forward<TArray<TSharedPtr<ChainedLocation>>>(chainedLocations));
    this->ProceduralSpaceShipBase->generateMesh(std::move(FName("Player")));
    this->ProceduralSpaceShipBase->SetMaterial(0, this->MatBase);
}

void AHub_Pawn::generateShell()
{
    FVector cubeSize = FVector(15.0f, 15.0f, 15.0f);
    this->ProceduralSpaceShipShell->setCubeSize(cubeSize);
    TArray<TSharedPtr<ChainedLocation>> chainedLocations;

    TArray<TSharedPtr<ChainedLocation>> const& chainedLocationBase = this->ProceduralSpaceShipBase->getEdges();
    TArray<FVector> locationBase;
    for (auto chained : chainedLocationBase)
    {
        if (chained)
        {
            locationBase.Add(chained->getCenter());
        }
    }

    int8 radius = 105;
    for (int8 x = -radius; x < radius; x += 15)
    {
        for (int8 y = -radius; y < radius; y += 15)
        {
            for (int8 z = -radius; z < radius; z += 15)
            {
                FVector loc = FVector(x, y, z);
                if (FVector::Dist(loc, FVector::ZeroVector) >= radius)
                {
                    continue;
                }

                if (locationBase.Find(loc) != INDEX_NONE)
                {
                    continue;
                }
                chainedLocations.Add(MakeShareable(new ChainedLocation(loc, cubeSize)));
            }
        }
    }

    this->ProceduralSpaceShipShell->setEdges(std::forward<TArray<TSharedPtr<ChainedLocation>>>(chainedLocations));
    this->ProceduralSpaceShipShell->generateMesh(std::move(FName("Player")));
    this->ProceduralSpaceShipShell->SetMaterial(0, this->MatShell);
}

void AHub_Pawn::generateEngine()
{
    FVector cubeSize = FVector(5.0f, 5.0f, 5.0f);
    this->ProceduralSpaceShipEngine->setCubeSize(cubeSize);
    int8 radius = 120;
    TArray<TSharedPtr<ChainedLocation>> chainedLocations =
    {
        MakeShareable(new ChainedLocation(FVector(-60.0f, -radius, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-45.0f, -radius, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-30.0f, -radius, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-15.0f, -radius, 0.0f), cubeSize)),

        MakeShareable(new ChainedLocation(FVector(-75.0f, -radius + 15.0f, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-90.0f, -radius + 15.0f, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-105.0f, -radius + 30.0f, 0.0f), cubeSize)),

        MakeShareable(new ChainedLocation(FVector(-60.0f, radius, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-45.0f, radius, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-30.0f, radius, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-15.0f, radius, 0.0f), cubeSize)),

        MakeShareable(new ChainedLocation(FVector(-75.0f, radius - 15.0f, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-90.0f, radius - 15.0f, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-105.0f, radius - 30.0f, 0.0f), cubeSize)),

        MakeShareable(new ChainedLocation(FVector(-120.0f, -radius + 30.0f, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, -radius + 45.0f, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, -radius + 60.0f, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, -radius + 60.0f, 15.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, -radius + 60.0f, -15.0f), cubeSize)),

        MakeShareable(new ChainedLocation(FVector(-120.0f, radius - 30.0f, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, radius - 45.0f, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, radius - 60.0f, 0.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, radius - 60.0f, 15.0f), cubeSize)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, radius - 60.0f, -15.0f), cubeSize)),
    };
    
    this->ProceduralSpaceShipEngine->setEdges(std::forward<TArray<TSharedPtr<ChainedLocation>>>(chainedLocations));
    this->ProceduralSpaceShipEngine->generateMesh(std::move(FName("Player")));
    this->ProceduralSpaceShipEngine->SetMaterial(0, MatEngine);
}

void AHub_Pawn::snapTarget(float _deltaTime)
{
    if (!this->m_isSnap)
    {
        // move actor to shell orientation
        if (this->ProceduralSpaceShipShell && this->m_progressResetSnap != 0.0f)
        {
            this->m_progressResetSnap = FMath::Clamp(this->m_progressResetSnap - _deltaTime, 0.0f, this->TimeToResetSnap);

            FRotator r1 = FMath::Lerp(this->m_defaultRotation, this->m_snapRelativeRotationOnRelease, this->m_progressResetSnap / this->TimeToResetSnap);
            FRotator r2 = this->ProceduralSpaceShipShell->GetRelativeTransform().Rotator() - r1;
            this->ProceduralSpaceShipShell->SetRelativeRotation(r1);
            AddActorLocalRotation(r2);
        }
    }
    else if (GEngine->GameViewport && GEngine->GameViewport->Viewport && this->ProceduralSpaceShipShell)
    {
        APlayerController const* playerController = Cast<APlayerController>(GetController());
        if (!playerController)
        {
            return;
        }

        FVector worldPosition, worldDirection;
        if (UGameplayStatics::DeprojectScreenToWorld(playerController, this->CrosshairPosition, worldPosition, worldDirection))
        {
            FVector const& actorLocation = GetActorLocation();
            FVector target = actorLocation + worldDirection * 1000.0f;
            DrawDebugSphere(GetWorld(), target, 300.0f, 18, FColor::Red);
            FRotator rot = UKismetMathLibrary::FindLookAtRotation(actorLocation, target);

            FRotator deltaRot = GetActorRotation();
            deltaRot += (rot - deltaRot) * SensibilitySnap;

            this->ProceduralSpaceShipShell->SetWorldRotation(rot);
        }
    }
}

void AHub_Pawn::resetCrosshair()
{
    if (GEngine->GameViewport && GEngine->GameViewport->Viewport)
    {
        this->m_viewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
        this->CrosshairPosition = this->m_viewportSize / 2.0f;
    }
}
