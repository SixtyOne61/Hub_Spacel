// Fill out your copyright notice in the Description page of Project Settings.


#include "Hub_Pawn.h"
#include "Source/Projectile/SimpleBullet.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "Source/Mesh/SpacelProceduralMeshComponent.h"
#include "Materials/MaterialInstance.h"
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

    ProceduralSpaceShipEngine = CreateDefaultSubobject<USpacelProceduralMeshComponent>(TEXT("ProceduralEngine0"));
    ProceduralSpaceShipEngine->bUseAsyncCooking = true;
    ProceduralSpaceShipEngine->SetupAttachment(RootComponent);

	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->SetupAttachment(RootComponent);	// Attach SpringArm to RootComponent
	SpringArm->TargetArmLength = 160.0f; // The camera follows at this distance behind the character	
	SpringArm->SocketOffset = FVector(0.f, 0.f, 60.f);
	SpringArm->bEnableCameraLag = true;	// Do not allow camera to lag
	SpringArm->CameraLagSpeed = 15.f;

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
	
	// bind function
	PlayerInputComponent->BindAxis("Speed", this, &AHub_Pawn::input_Speed);
	PlayerInputComponent->BindAxis("MoveUp", this, &AHub_Pawn::input_MoveUp);
	PlayerInputComponent->BindAxis("MoveRight", this, &AHub_Pawn::input_MoveRight);
	PlayerInputComponent->BindAxis("MoveRoll", this, &AHub_Pawn::input_MoveRoll);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AHub_Pawn::input_Fire);
	PlayerInputComponent->BindAction("HandBrake", IE_Pressed, this, &AHub_Pawn::input_HandBrakePress);
}

void AHub_Pawn::input_Fire()
{
	if (Role < ROLE_Authority)
	{
		server_Fire();
		return;
	}

    FTransform transform = ProceduralSpaceShipBase->GetSocketTransform("SimpleBulletSpawn");
    ASimpleBullet* pBullet = Cast<ASimpleBullet>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), SimpleBulletClass, transform));
    if (pBullet)
    {
        // TO DO init bullet
        pBullet->SetReplicates(true);
        UGameplayStatics::FinishSpawningActor(pBullet, transform);
        pBullet->netMulticast_launchBullet(GetActorForwardVector());
    }
}

void AHub_Pawn::input_Speed(float _val)
{
	// Is there any input?
	bool bHasInput = !FMath::IsNearlyEqual(_val, 0.f);
	// If input is not held down, reduce speed
	float currentAcc = bHasInput ? (_val * m_acceleration) : (-0.5f * m_acceleration);
	// Calculate new speed
	float newForwardSpeed = m_currentForwardSpeed + (GetWorld()->GetDeltaSeconds() * currentAcc);
	// Clamp between MinSpeed and MaxSpeed
	m_currentForwardSpeed = FMath::Clamp(newForwardSpeed, m_minSpeed, m_maxSpeed);	
}

void AHub_Pawn::input_MoveUp(float _val)
{
	// target pitch speed is based in input
	float targetPitchSpeed = (_val * m_turnSpeed * -1.0f);

	// when steering, we decrease pitch slightly
	targetPitchSpeed += (FMath::Abs(m_currentYawSpeed) * -0.2f);

	// Smoothly interpolate to target pitch speed
	m_currentPitchSpeed = FMath::FInterpTo(m_currentPitchSpeed, targetPitchSpeed, GetWorld()->GetDeltaSeconds(), m_interpSpeed);
}

void AHub_Pawn::input_MoveRight(float _val)
{
	// target yaw speed is based on input
	float targetYawSpeed = (_val * m_turnSpeed);

	// smoothly interpolate to target yaw speed
	m_currentYawSpeed = FMath::FInterpTo(m_currentYawSpeed, targetYawSpeed, GetWorld()->GetDeltaSeconds(), m_interpSpeed);

	// Is there any left / right input ?
	const bool isTurning = FMath::Abs(_val) > 0.2f;

	// if turning, yaw value is used to influence rool
	// if not turning, roll to reverse current roll value
	float targetRollSpeed = isTurning ? (m_currentYawSpeed * 0.8f) : (GetActorRotation().Roll * -2.0f);

	// smoothly interpolate roll speed
	m_currentRollSpeed = FMath::FInterpTo(m_currentRollSpeed, targetRollSpeed, GetWorld()->GetDeltaSeconds(), m_interpSpeed);
}

void AHub_Pawn::input_MoveRoll(float _val)
{
	// target roll speed is based on input
	float targetRollSpeed = (_val * m_rollSpeed);

	// smoothly interpolate to target roll speed
	m_currentRollSpeed = FMath::FInterpTo(m_currentRollSpeed, targetRollSpeed, GetWorld()->GetDeltaSeconds(), m_interpSpeed);
}

void AHub_Pawn::input_HandBrakePress()
{

}

void AHub_Pawn::server_Fire_Implementation()
{
	input_Fire();
}

bool AHub_Pawn::server_Fire_Validate()
{
	return true;
}

void AHub_Pawn::generateMesh()
{
    FVector const& location = this->GetActorLocation();

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
        MakeShareable(new ChainedLocation(FVector(-15,15,0), 15.0f)),
        MakeShareable(new ChainedLocation(FVector(-15,-15,0), 15.0f)),
    };
    ProceduralSpaceShipBase->setEdges(std::forward<TArray<TSharedPtr<ChainedLocation>>>(chainedLocations));
    ProceduralSpaceShipBase->generateMesh();
    ProceduralSpaceShipBase->SetMaterial(0, MatBase);
}

void AHub_Pawn::generateEngine()
{
    ProceduralSpaceShipEngine->setCubeSize(5.0f);
    TArray<TSharedPtr<ChainedLocation>> chainedLocations =
    {
        MakeShareable(new ChainedLocation(FVector(-25.0f, -15.0f, 0.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-25.0f, -10.0f, 0.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-25.0f, -5.0f, 0.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-25.0f, 0.0f, 0.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-25.0f, 5.0f, 0.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-25.0f, 10.0f, 0.0f), 5.0f)),
        MakeShareable(new ChainedLocation(FVector(-25.0f, 15.0f, 0.0f), 5.0f)),
    };
    ProceduralSpaceShipEngine->setEdges(std::forward<TArray<TSharedPtr<ChainedLocation>>>(chainedLocations));
    ProceduralSpaceShipEngine->generateMesh();
    ProceduralSpaceShipEngine->SetMaterial(0, MatEngine);
}
