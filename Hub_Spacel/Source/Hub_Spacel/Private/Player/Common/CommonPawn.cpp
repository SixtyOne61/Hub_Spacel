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
#include "Mesh/SpacelInstancedMeshComponent.h"
#include "Mesh/EmergencyInstancedMeshComponent.h"
#include "Player/TargetActor.h"
#include "Player/FireComponent.h"
#include "Player/ModuleComponent.h"
#include "Player/CustomCollisionComponent.h"
#include "Player/RepairComponent.h"
#include "Player/LocalPlayerActionComponent.h"
#include "Player/PlayerNameActor.h"
#include "Player/MetricComponent.h"
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

    EmergencyComponent = CreateDefaultSubobject<UEmergencyInstancedMeshComponent>(TEXT("Emergency_00"));
    if (!ensure(EmergencyComponent != nullptr)) return;
    EmergencyComponent->SetupAttachment(ModuleComponent);

    WeaponComponent = CreateDefaultSubobject<USpacelInstancedMeshComponent>(TEXT("Weapon_00"));
    if (!ensure(WeaponComponent != nullptr)) return;
    WeaponComponent->SetupAttachment(ModuleComponent);

    ProtectionComponent = CreateDefaultSubobject<USpacelInstancedMeshComponent>(TEXT("Protection_00"));
    if (!ensure(ProtectionComponent != nullptr)) return;
    ProtectionComponent->SetupAttachment(ModuleComponent);

    SupportComponent = CreateDefaultSubobject<USpacelInstancedMeshComponent>(TEXT("Support_00"));
    if (!ensure(SupportComponent != nullptr)) return;
    SupportComponent->SetupAttachment(ModuleComponent);

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

    MetricComponent = CreateDefaultSubobject<UMetricComponent>(TEXT("Metric_00"));
    if(!ensure(MetricComponent != nullptr)) return;

    Tags.Add(Tags::Player);
}

void ACommonPawn::lookAt(FVector const& _loc, FVector const& _dir, FVector const& _hitLoc)
{
    FVector farpoint = _loc + _dir * 3000;
    FVector bigFar = _loc + _dir * 100000;
    this->TargetLocation = _hitLoc.IsNearlyZero() ? bigFar : _hitLoc;

    FRotator rotation = SimplyMath::MyLookRotation(bigFar, this->GetActorUpVector(), this->GetActorLocation());
    setActorRotation(rotation);
}

// Called when the game starts or when spawned
void ACommonPawn::BeginPlay()
{
	Super::BeginPlay();

    auto lb = [&](FName _tag) -> UNiagaraComponent*
    {
        TArray<UActorComponent*> out;
        out = this->GetComponentsByTag(UNiagaraComponent::StaticClass(), "RightTrail");
        for (auto comp : out)
        {
            if (comp != nullptr)
            {
                return Cast<UNiagaraComponent>(comp);
            }
        }

        return nullptr;
    };

    RightTrailComponent = lb("RightTrail");
    LeftTrailComponent = lb("LeftTrail");
}

void ACommonPawn::OnRep_RightTrail()
{
    if (this->RightTrailComponent == nullptr) return;
    FVector dirRight { 0.0f, 1.0f, 0.0f };
    this->RightTrailComponent->SetActive(RU_RightTrail, true);
    this->RightTrailComponent->SetNiagaraVariableVec3("User.Velocity", RU_RightTrail ? dirRight : FVector::ZeroVector);
}

void ACommonPawn::OnRep_LeftTrail()
{
    if (this->LeftTrailComponent == nullptr) return;
    FVector dirLeft { 0.0f, -1.0f, 0.0f };
    this->RightTrailComponent->SetActive(RU_LeftTrail, true);
    this->LeftTrailComponent->SetNiagaraVariableVec3("User.Velocity", RU_LeftTrail ? dirLeft : FVector::ZeroVector);
}

void ACommonPawn::moveShip(float _deltaTime)
{
    if (!ensure(this->DriverMeshComponent != nullptr)) return;
    if (!ensure(this->PlayerDataAsset != nullptr)) return;

    // stun
    if(hasEffect(EEffect::Emp)) return;

    // 9, default support size
    float coefSpeed = FMath::Max((this->SupportComponent->GetNum() / 9.0f), this->PlayerDataAsset->MinCoefSpeed);
    if (hasEffect(EEffect::MetaFormAttack) || hasEffect(EEffect::MetaFormProtection) || hasEffect(EEffect::MetaFormSupport) || hasEffect(EEffect::EscapeMode))
    {
        // override speed max
        coefSpeed = this->PlayerDataAsset->EscapeModeCoef;
    }

    coefSpeed += (m_bonusSpeed / 100.0f);

    // roll rotation
    FRotator rotation = this->GetActorRotation();
    rotation.Add(0.0f, 0.0f, this->PercentFlightAttitude * FMath::Max(coefSpeed, 1.0f) * this->PlayerDataAsset->MaxFlightAttitudeSpeed);
    setActorRotation(rotation);

    // missile only move rotation
    if (hasEffect(EEffect::Missile))
    {
        this->DriverMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
        this->DriverMeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        return;
    }

    // linear
    FVector const& linearVelocity = this->DriverMeshComponent->GetPhysicsLinearVelocity(NAME_None);

    float percentSpeed = this->R_OverDrive != 0.0f ? (this->R_OverDrive / 10.0f) : this->RU_PercentSpeed;
    FVector newVelocity = this->DriverMeshComponent->GetForwardVector() * this->PlayerDataAsset->MaxForwardSpeed * percentSpeed * coefSpeed;
    if (hasEffect(EEffect::EscapeMode))
    {
        newVelocity += this->DriverMeshComponent->GetRightVector() * this->PlayerDataAsset->MaxHorizontalSpeed * this->PercentHorizontalStraf * (coefSpeed * 2);
    }
    else
    {
        newVelocity += this->DriverMeshComponent->GetRightVector() * this->PlayerDataAsset->MaxHorizontalSpeed * this->PercentHorizontalStraf * coefSpeed;
    }

    newVelocity += this->DriverMeshComponent->GetUpVector() * this->PlayerDataAsset->MaxVerticalSpeed * this->PercentVerticalStraf * coefSpeed;
    newVelocity = FMath::Lerp(linearVelocity, newVelocity, 0.9f);

    this->DriverMeshComponent->SetPhysicsLinearVelocity(newVelocity);
    this->DriverMeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
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
    if (this->ExhaustFxComponents.Num() == 0)
    {
        TArray<UActorComponent*> out;
        this->GetComponents(UNiagaraComponent::StaticClass(), out);
        for (auto com : out)
        {
            if (com != nullptr && com->GetFName().ToString().Contains("Exhaust"))
            {
                this->ExhaustFxComponents.Add(Cast<UNiagaraComponent>(com));
            }
        }
    }

    for (auto exhaust : this->ExhaustFxComponents)
    {
        if (exhaust != nullptr && exhaust->IsActive())
        {
            exhaust->SetNiagaraVariableFloat("User.Velocity", this->RU_PercentSpeed);
        }
    }
}

void ACommonPawn::setCollisionProfile(FString _team)
{
    if (!ensure(this->DriverMeshComponent != nullptr)) return;

    _team = _team.Replace(TEXT(" "), TEXT(""));
    this->DriverMeshComponent->SetCollisionProfileName(*_team);

    this->ModuleComponent->setCollisionProfile(_team);
}

void ACommonPawn::RPCClientDamageIndicator_Implementation(FVector_NetQuantize const& _location)
{
    if (this->IsLocallyControlled())
    {
        BP_DamageIndicator(_location);
    }
}

void ACommonPawn::halfTurn()
{
    FVector const& loc = this->GetActorLocation();
    FVector dir = this->GetActorForwardVector() * -1.0f;

    FVector farpoint = loc + dir * 100000;

    FRotator rotation = SimplyMath::MyLookRotation(farpoint, this->GetActorUpVector(), this->GetActorLocation());
    this->SetActorRotation(rotation);

    m_freezeRotation = true;
    FTimerHandle handle;
    this->GetWorldTimerManager().SetTimer(handle, this, &ACommonPawn::UnFreezeRotation, 1.0f, false);
}

void ACommonPawn::UnFreezeRotation()
{
    m_freezeRotation = false;
}

void ACommonPawn::setActorRotation(FRotator const& _rotator)
{
    if (!m_freezeRotation)
    {
        this->SetActorRotation(_rotator);
    }
}

void ACommonPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ACommonPawn, R_Effect);
    DOREPLIFETIME(ACommonPawn, RU_PercentSpeed);
    DOREPLIFETIME(ACommonPawn, R_OverDrive);
    DOREPLIFETIME(ACommonPawn, RU_RightTrail);
    DOREPLIFETIME(ACommonPawn, RU_LeftTrail);
    DOREPLIFETIME(ACommonPawn, R_BonusCountDown);
}

