// Fill out your copyright notice in the Description page of Project Settings.


#include "CommonPawn.h"
#include "Util/SimplyMath.h"
#include "Util/Tag.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Player/TargetActor.h"
#include "Player/FireComponent.h"
#include "Player/ModuleComponent.h"
#include "Player/CustomCollisionComponent.h"
#include "Player/RepairComponent.h"
#include "Player/LocalPlayerActionComponent.h"
#include "Player/PlayerNameActor.h"
#include "DataAsset/PlayerDataAsset.h"
#include "DataAsset/TeamColorDataAsset.h"
#include "Gameplay/SkillComponent.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraComponent.h"

// Sets default values
ACommonPawn::ACommonPawn()
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

    ModuleComponent = CreateDefaultSubobject<UModuleComponent>(TEXT("Module_00"));
    if (!ensure(ModuleComponent != nullptr)) return;
    ModuleComponent->SetupAttachment(BaseShipMeshComponent);

    // Create a spring arm component
    SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm_00"));
    if (!ensure(SpringArmComponent != nullptr)) return;
    SpringArmComponent->SetupAttachment(BaseShipMeshComponent);

    // Create camera component 
    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera_00"));
    if (!ensure(CameraComponent != nullptr)) return;
    CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName); // Attach the camera

    FireComponent = CreateDefaultSubobject<UFireComponent>(TEXT("Fire_00"));
    if (!ensure(FireComponent != nullptr)) return;
    FireComponent->Deactivate();

    SkillComponent = CreateDefaultSubobject<USkillComponent>(TEXT("Skill_00"));
    if (!ensure(SkillComponent != nullptr)) return;
    SkillComponent->Deactivate();

    RepairComponent = CreateDefaultSubobject<URepairComponent>(TEXT("Repair_00"));
    if (!ensure(RepairComponent != nullptr)) return;
    RepairComponent->Deactivate();

    TargetComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("Target_00"));
    if (!ensure(TargetComponent != nullptr)) return;
    TargetComponent->SetupAttachment(RootComponent);

    PlayerNameComponent = CreateDefaultSubobject<UChildActorComponent>(TEXT("PlayerName_00"));
    if (!ensure(PlayerNameComponent != nullptr)) return;
    PlayerNameComponent->SetupAttachment(RootComponent);

    SpeedLinesComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("SpeedLines_00"));
    if (!ensure(SpeedLinesComponent != nullptr)) return;

    ShieldComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shield_00"));
    if (!ensure(ShieldComponent != nullptr)) return;
    ShieldComponent->SetupAttachment(RootComponent);

    Tags.Add(Tags::Player);
}

void ACommonPawn::lookAt(FVector const& _loc, FVector const& _dir, FVector const& _hitLoc)
{
    FVector farpoint = _loc + _dir * 3000;
    FVector bigFar = _loc + _dir * 100000;
    this->TargetLocation = _hitLoc.IsNearlyZero() ? bigFar : _hitLoc;

    FRotator rotation = SimplyMath::MyLookRotation(bigFar, this->GetActorUpVector(), this->GetActorLocation());
    this->SetActorRotation(rotation);
}

// Called when the game starts or when spawned
void ACommonPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

void ACommonPawn::moveShip(float _deltaTime)
{
    if (!ensure(this->DriverMeshComponent != nullptr)) return;
    if (!ensure(this->PlayerDataAsset != nullptr)) return;
    if (!ensure(this->ModuleComponent != nullptr)) return;
    if (!ensure(this->ModuleComponent->SupportMeshComponent != nullptr)) return;

    // 9, default support size
    float coefSpeed = FMath::Max((this->ModuleComponent->SupportMeshComponent->GetInstanceCount() / 9.0f), this->PlayerDataAsset->MinCoefSpeed);
    if (hasEffect(EEffect::MetaFormAttack) || hasEffect(EEffect::MetaFormProtection) || hasEffect(EEffect::MetaFormSupport) || hasEffect(EEffect::EscapeMode))
    {
        // override speed max
        coefSpeed = this->PlayerDataAsset->EscapeModeCoef;
    }

    // roll rotation
    FRotator rotation = this->GetActorRotation();
    rotation.Add(0.0f, 0.0f, this->PercentFlightAttitude * FMath::Max(coefSpeed, 1.0f) * this->PlayerDataAsset->MaxFlightAttitudeSpeed);
    this->SetActorRotation(rotation);

    // linear
    FVector const& linearVelocity = this->DriverMeshComponent->GetPhysicsLinearVelocity(NAME_None);

    float percentSpeed = this->R_OverDrive != 0.0f ? this->R_OverDrive : this->RU_PercentSpeed;
    FVector newVelocity = this->DriverMeshComponent->GetForwardVector() * this->PlayerDataAsset->MaxForwardSpeed * percentSpeed * coefSpeed;
    newVelocity += this->DriverMeshComponent->GetRightVector() * this->PlayerDataAsset->MaxHorizontalSpeed * this->PercentHorizontalStraf * coefSpeed;
    newVelocity += this->DriverMeshComponent->GetUpVector() * this->PlayerDataAsset->MaxVerticalSpeed * this->PercentVerticalStraf * coefSpeed;
    newVelocity = FMath::Lerp(linearVelocity, newVelocity, 0.9f);

    this->DriverMeshComponent->SetPhysicsLinearVelocity(newVelocity);
}

bool ACommonPawn::hasEffect(EEffect _type)
{
    return (this->R_Effect & TOFLAG(_type));
}

void ACommonPawn::addEffect(EEffect _type)
{
    if (!hasEffect(_type))
    {
        this->R_Effect |= TOFLAG(_type);
        addEffectSuccess(_type);
    }
}

void ACommonPawn::removeEffect(EEffect _type)
{
    if (hasEffect(_type))
    {
        this->R_Effect &= ~TOFLAG(_type);
        removeEffectSuccess(_type);
    }
}

void ACommonPawn::OnRep_PercentSpeed()
{
    if (this->ExhaustFxComponent == nullptr)
    {
        this->ExhaustFxComponent = Cast<UNiagaraComponent>(this->GetComponentByClass(UNiagaraComponent::StaticClass()));
    }

    if (this->ExhaustFxComponent != nullptr)
    {
        this->ExhaustFxComponent->SetNiagaraVariableFloat("User.Velocity", this->RU_PercentSpeed);
    }
}

void ACommonPawn::setCollisionProfile(FString _team)
{
    if (!ensure(this->DriverMeshComponent != nullptr)) return;

    _team = _team.Replace(TEXT(" "), TEXT(""));
    this->DriverMeshComponent->SetCollisionProfileName(*_team);

    this->ModuleComponent->setCollisionProfile(_team);
}

void ACommonPawn::addPlayerFocusOnMe(int32 _playerId)
{
    PlayerFocusOnYou.Add(_playerId);
    if (!hasEffect(EEffect::Targeted))
    {
        addEffect(EEffect::Targeted);
    }
}

void ACommonPawn::removePlayerFocusOnMe(int32 _playerId)
{
    PlayerFocusOnYou.Remove(_playerId);
    if (PlayerFocusOnYou.Num() == 0)
    {
        removeEffect(EEffect::Targeted);
    }
}

void ACommonPawn::removeAllPlayerFocusOnMe()
{
    PlayerFocusOnYou.Empty();
    removeEffect(EEffect::Targeted);
}

void ACommonPawn::RPCClientDamageIndicator_Implementation(FVector const& _location)
{
    if (this->IsLocallyControlled())
    {
        BP_DamageIndicator(_location);
    }
}

void ACommonPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACommonPawn, R_Effect);
    DOREPLIFETIME(ACommonPawn, RU_PercentSpeed);
    DOREPLIFETIME(ACommonPawn, R_OverDrive);
}

