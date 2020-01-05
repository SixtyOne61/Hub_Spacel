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

// Sets default values
AHub_Pawn::AHub_Pawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create static mesh component
	SpaceShipMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SpaceShip"));
	RootComponent = SpaceShipMesh;

	// Create a spring arm component
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->SetupAttachment(RootComponent);	// Attach SpringArm to RootComponent
	SpringArm->TargetArmLength = 160.0f; // The camera follows at this distance behind the character	
	SpringArm->SocketOffset = FVector(0.f, 0.f, 60.f);
	SpringArm->bEnableCameraLag = false;	// Do not allow camera to lag
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
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AHub_Pawn::input_Fire);
	PlayerInputComponent->BindAxis("Speed", this, &AHub_Pawn::input_Speed);
	PlayerInputComponent->BindAxis("MoveUp", this, &AHub_Pawn::input_MoveUp);
	PlayerInputComponent->BindAxis("MoveRight", this, &AHub_Pawn::input_MoveRight);
}

void AHub_Pawn::input_Fire()
{
	if (Role < ROLE_Authority)
	{
		server_Fire();
		return;
	}

	UStaticMeshSocket const* socket = SpaceShipMesh->GetSocketByName("SimpleBulletSpawn");
	if (!SimpleBulletClass || !socket)
	{
		return;
	}

	FTransform transform;
	if (socket->GetSocketTransform(transform, SpaceShipMesh))
	{
		ASimpleBullet* pBullet = Cast<ASimpleBullet>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), SimpleBulletClass, transform));
		if (pBullet)
		{
			// TO DO init bullet
			pBullet->SetReplicates(true);
			UGameplayStatics::FinishSpawningActor(pBullet, transform);
			pBullet->netMulticast_launchBullet(GetActorForwardVector());
		}
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
	// TO DO
}

void AHub_Pawn::server_Fire_Implementation()
{
	input_Fire();
}

bool AHub_Pawn::server_Fire_Validate()
{
	return true;
}
