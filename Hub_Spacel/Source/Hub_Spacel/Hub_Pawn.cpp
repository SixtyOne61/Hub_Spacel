// Fill out your copyright notice in the Description page of Project Settings.


#include "Hub_Pawn.h"
#include "Source/Projectile/SimpleBullet.h"
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
    if (ProceduralSpaceShipShell)
    {
        m_defaultRotation = ProceduralSpaceShipShell->GetRelativeTransform().Rotator();
    }
}

// Called every frame
void AHub_Pawn::Tick(float DeltaTime)
{
	// move
	const FVector localMove = FVector(m_currentForwardSpeed * DeltaTime, 0.f, 0.f);

	// Move plan forwards (with sweep so we stop when we collide with things)
	AddActorLocalOffset(localMove, true);

	// rotation
	FRotator deltaRotation(0.f, 0.f, 0.f);
	deltaRotation.Pitch = m_currentPitchSpeed * DeltaTime;
	deltaRotation.Yaw = m_currentYawSpeed * DeltaTime;
	deltaRotation.Roll = m_currentRollSpeed * DeltaTime;

	// rotate ship
	AddActorLocalRotation(deltaRotation);

    snapTarget(DeltaTime);
    fireLaser(DeltaTime);

	// Call any parent class Tick implementation
	Super::Tick(DeltaTime);
}

void AHub_Pawn::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	// Deflect along the surface when we collide.
	FRotator CurrentRotation = GetActorRotation();
	SetActorRotation(FQuat::Slerp(CurrentRotation.Quaternion(), HitNormal.ToOrientationQuat(), 0.025f));
}

// Called to bind functionality to input
void AHub_Pawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Check if PlayerInputComponent is valid (not NULL)
	check(PlayerInputComponent);
	
    /* bind input rework */
    PlayerInputComponent->BindAxis("MoveUp", this, &AHub_Pawn::input_MoveUp);
    PlayerInputComponent->BindAxis("MoveRight", this, &AHub_Pawn::input_MoveRight);
    PlayerInputComponent->BindAxis("MoveTargetUp", this, &AHub_Pawn::input_MoveTargetUp);
    PlayerInputComponent->BindAxis("MoveTargetRight", this, &AHub_Pawn::input_MoveTargetRight);
    PlayerInputComponent->BindAxis("Speed", this, &AHub_Pawn::input_Speed);

    PlayerInputComponent->BindAction("Snap", IE_Pressed, this, &AHub_Pawn::input_SnapOn);
    PlayerInputComponent->BindAction("Snap", IE_Released, this, &AHub_Pawn::input_SnapOff);
    PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AHub_Pawn::input_FireOn);
    PlayerInputComponent->BindAction("Fire", IE_Released, this, &AHub_Pawn::input_FireOff);
}

void AHub_Pawn::input_FireOn()
{
    m_isFire = true;
}

void AHub_Pawn::input_FireOff()
{
    m_isFire = false;
}

void AHub_Pawn::input_Speed(float _val)
{
    // TO DO : not cool + _val
    PercentSpeed = FMath::Clamp(PercentSpeed + _val, 0.0f, 100.0f);
    m_currentForwardSpeed = MaxForwardSpeed * (PercentSpeed / 100.0f);
}

void AHub_Pawn::input_MoveUp(float _val)
{
	// target pitch speed is based in input
	float targetPitchSpeed = (_val * TurnSpeed * -1.0f);

	// when steering, we decrease pitch slightly
	targetPitchSpeed += (FMath::Abs(m_currentYawSpeed) * -0.2f);

	// Smoothly interpolate to target pitch speed
	m_currentPitchSpeed = FMath::FInterpTo(m_currentPitchSpeed, targetPitchSpeed, GetWorld()->GetDeltaSeconds(), InterpSpeed);
}

void AHub_Pawn::input_MoveRight(float _val)
{
	// target yaw speed is based on input
	float targetYawSpeed = (_val * TurnSpeed);

	// smoothly interpolate to target yaw speed
	m_currentYawSpeed = FMath::FInterpTo(m_currentYawSpeed, targetYawSpeed, GetWorld()->GetDeltaSeconds(), InterpSpeed);

	// Is there any left / right input ?
	const bool isTurning = FMath::Abs(_val) > 0.2f;

	// if turning, yaw value is used to influence rool
	// if not turning, roll to reverse current roll value
	float targetRollSpeed = isTurning ? (m_currentYawSpeed * 0.8f) : (GetActorRotation().Roll * -2.0f);

	// smoothly interpolate roll speed
	m_currentRollSpeed = FMath::FInterpTo(m_currentRollSpeed, targetRollSpeed, GetWorld()->GetDeltaSeconds(), InterpSpeed);
}

void AHub_Pawn::input_MoveTargetUp(float _val)
{
    if (!m_isSnap)
    {
        return;
    }

    float sensibility = SensibilityCrosshair;
    float delta = m_viewportSize.X * _val * sensibility;
    CrosshairPosition.X = FMath::Clamp(CrosshairPosition.X + delta, 0.0f, m_viewportSize.X);
}

void AHub_Pawn::input_MoveTargetRight(float _val)
{
    if (!m_isSnap)
    {
        return;
    }

    float sensibility = SensibilityCrosshair;
    float delta = m_viewportSize.Y * _val * sensibility;
    CrosshairPosition.Y = FMath::Clamp(CrosshairPosition.Y + delta, 0.0f, m_viewportSize.Y);
}

void AHub_Pawn::input_SnapOn()
{
    m_isSnap = true;
}

void AHub_Pawn::input_SnapOff()
{
    m_isSnap = false;
    m_progressResetSnap = TimeToResetSnap;
    if (ProceduralSpaceShipShell)
    {
        m_snapRotationOnRelease = ProceduralSpaceShipShell->GetComponentRotation();
        m_snapRelativeRotationOnRelease = ProceduralSpaceShipShell->GetRelativeTransform().Rotator();
    }

    resetCrosshair();
}

void AHub_Pawn::fireLaser(float _deltaTime)
{
    if (m_isFire && m_laserCountDown <= 0.0f)
    {
        FTransform transform = ProceduralSpaceShipBase->GetSocketTransform("SimpleBulletSpawn");
        AActor* pLaser = Cast<AActor>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), LaserClass, transform));
        if (pLaser)
        {
            // TO DO init bullet
            pLaser->SetReplicates(true);
            UGameplayStatics::FinishSpawningActor(pLaser, transform);
            UProjectileMovementComponent* comp = Cast<UProjectileMovementComponent>(pLaser->GetComponentByClass(UProjectileMovementComponent::StaticClass()));
            if (comp && ProceduralSpaceShipShell)
            {
                comp->SetVelocityInLocalSpace(FVector(1, 0, 0) * comp->InitialSpeed);
            }
        }
        m_laserCountDown = TimeBetweenLaserShot;
    }
    else if (m_laserCountDown != 0.0f)
    {
        m_laserCountDown -= _deltaTime;
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

    lb_init(ProceduralSpaceShipBase, std::bind(&AHub_Pawn::generateBase, this));
    lb_init(ProceduralSpaceShipShell, std::bind(&AHub_Pawn::generateShell, this));
    lb_init(ProceduralSpaceShipEngine, std::bind(&AHub_Pawn::generateEngine, this));
}

void AHub_Pawn::generateBase()
{
    ProceduralSpaceShipBase->setCubeSize(15.0f);
    TArray<TSharedPtr<ChainedLocation>> chainedLocations =
    {
        MakeShareable(new ChainedLocation(FVector(-15,0,0), 15.0f)),
        MakeShareable(new ChainedLocation(FVector(0,0,0), 15.0f)),
        MakeShareable(new ChainedLocation(FVector(15,0,0), 15.0f)),
        MakeShareable(new ChainedLocation(FVector(0,15,0), 15.0f)),
        MakeShareable(new ChainedLocation(FVector(0,-15,0), 15.0f)),
        MakeShareable(new ChainedLocation(FVector(0,0,15), 15.0f)),
        MakeShareable(new ChainedLocation(FVector(0,0,-15), 15.0f)),
    };
    ProceduralSpaceShipBase->setEdges(std::forward<TArray<TSharedPtr<ChainedLocation>>>(chainedLocations));
    ProceduralSpaceShipBase->generateMesh(std::move(FName("Player")));
    ProceduralSpaceShipBase->SetMaterial(0, MatBase);
}

void AHub_Pawn::generateShell()
{
    ProceduralSpaceShipShell->setCubeSize(15.0f);
    TArray<TSharedPtr<ChainedLocation>> chainedLocations;

    TArray<TSharedPtr<ChainedLocation>> const& chainedLocationBase = ProceduralSpaceShipBase->getEdges();
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
                chainedLocations.Add(MakeShareable(new ChainedLocation(std::move(loc), 15.0f)));
            }
        }
    }

    ProceduralSpaceShipShell->setEdges(std::forward<TArray<TSharedPtr<ChainedLocation>>>(chainedLocations));
    ProceduralSpaceShipShell->generateMesh(std::move(FName("Player")));
    ProceduralSpaceShipShell->SetMaterial(0, MatShell);
}

void AHub_Pawn::generateEngine()
{
    ProceduralSpaceShipEngine->setCubeSize(15.0f);
    int8 radius = 120;
    TArray<TSharedPtr<ChainedLocation>> chainedLocations =
    {
        MakeShareable(new ChainedLocation(FVector(-60.0f, -radius, 0.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-45.0f, -radius, 0.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-30.0f, -radius, 0.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-15.0f, -radius, 0.0f), 5.0f)),

        MakeShareable(new ChainedLocation(FVector(-75.0f, -radius + 15.0f, 0.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-90.0f, -radius + 15.0f, 0.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-105.0f, -radius + 30.0f, 0.0f), 5.0f)),

        MakeShareable(new ChainedLocation(FVector(-60.0f, radius, 0.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-45.0f, radius, 0.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-30.0f, radius, 0.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-15.0f, radius, 0.0f), 5.0f)),

        MakeShareable(new ChainedLocation(FVector(-75.0f, radius - 15.0f, 0.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-90.0f, radius - 15.0f, 0.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-105.0f, radius - 30.0f, 0.0f), 5.0f)),

        MakeShareable(new ChainedLocation(FVector(-120.0f, -radius + 30.0f, 0.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, -radius + 45.0f, 0.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, -radius + 60.0f, 0.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, -radius + 60.0f, 15.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, -radius + 60.0f, -15.0f), 5.0f)),

        MakeShareable(new ChainedLocation(FVector(-120.0f, radius - 30.0f, 0.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, radius - 45.0f, 0.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, radius - 60.0f, 0.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, radius - 60.0f, 15.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-120.0f, radius - 60.0f, -15.0f), 5.0f)),
    };
    ProceduralSpaceShipEngine->setEdges(std::forward<TArray<TSharedPtr<ChainedLocation>>>(chainedLocations));
    ProceduralSpaceShipEngine->generateMesh(std::move(FName("Player")));
    ProceduralSpaceShipEngine->SetMaterial(0, MatEngine);
}

void AHub_Pawn::snapTarget(float _deltaTime)
{
    if (!m_isSnap)
    {
        // move actor to shell orientation
        if (ProceduralSpaceShipShell && m_progressResetSnap != 0.0f)
        {
            m_progressResetSnap = FMath::Clamp(m_progressResetSnap - _deltaTime, 0.0f, TimeToResetSnap);

            FRotator r1 = FMath::Lerp(m_defaultRotation, m_snapRelativeRotationOnRelease, m_progressResetSnap / TimeToResetSnap);
            FRotator r2 = ProceduralSpaceShipShell->GetRelativeTransform().Rotator() - r1;
            ProceduralSpaceShipShell->SetRelativeRotation(r1);
            AddActorLocalRotation(r2);
        }
    }
    else if (GEngine->GameViewport && GEngine->GameViewport->Viewport && ProceduralSpaceShipShell)
    {
        APlayerController const* playerController = Cast<APlayerController>(GetController());
        if (!playerController)
        {
            return;
        }

        FVector worldPosition, worldDirection;
        if (UGameplayStatics::DeprojectScreenToWorld(playerController, CrosshairPosition, worldPosition, worldDirection))
        {
            FVector const& actorLocation = GetActorLocation();
            FVector target = actorLocation + worldDirection * 1000.0f;
            DrawDebugSphere(GetWorld(), target, 300.0f, 18, FColor::Red);
            FRotator rot = UKismetMathLibrary::FindLookAtRotation(actorLocation, target);

            FRotator deltaRot = GetActorRotation();
            deltaRot += (rot - deltaRot) * SensibilitySnap;

            ProceduralSpaceShipShell->SetWorldRotation(rot);
        }
    }
}

void AHub_Pawn::resetCrosshair()
{
    if (GEngine->GameViewport && GEngine->GameViewport->Viewport)
    {
        m_viewportSize = FVector2D(GEngine->GameViewport->Viewport->GetSizeXY());
        CrosshairPosition = m_viewportSize / 2.0f;
    }
}
