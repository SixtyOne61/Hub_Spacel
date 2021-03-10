// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipPawn.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DataAsset/StaticMeshDataAsset.h"
#include "DataAsset/SetupAttributeDataAsset.h"
#include "DataAsset/PlayerDataAsset.h"
#include "Player/SpacelPlayerState.h"
#include "Player/TargetActor.h"
#include "Player/FireComponent.h"
#include "Player/ModuleComponent.h"
#include "Player/CustomCollisionComponent.h"
#include "Player/GamePlayerController.h"
#include "Player/RepairComponent.h"
#include "Player/LocalPlayerActionComponent.h"
#include "GameState/SpacelGameState.h"
#include "Hub_SpacelGameInstance.h"
#include "Util/Tag.h"
#include "Util/SimplyMath.h"
#include "TimerManager.h"
#include "NiagaraComponent.h"
#include "Gameplay/SkillComponent.h"

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

    SpeedLinesComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("SpeedLines_00"));
    if (!ensure(TargetComponent != nullptr)) return;

    Tags.Add(Tags::Player);
}

void AShipPawn::OnStartGame()
{
    // add custom collision component
    if (UCustomCollisionComponent* customCollisionComponent = NewObject<UCustomCollisionComponent>(this, "CustomCollision_00"))
    {
        customCollisionComponent->RegisterComponent();
    }

    if (this->SkillComponent != nullptr)
    {
        this->SkillComponent->setupSkill();
    }

    RPCClientStartGame(this->Team);
}

void AShipPawn::RPCClientStartGame_Implementation(FName const& _team)
{
    if (this->IsLocallyControlled())
    {
        this->OnLocalTeamUpdateDelegate.Broadcast(_team.ToString());
    }

    this->Team = _team;
}

// Called when the game starts or when spawned
void AShipPawn::BeginPlay()
{
	Super::BeginPlay();

    if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        ASpacelGameState* spacelGameState{ Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())) };
        if (spacelGameState != nullptr)
        {
            spacelGameState->OnStartGameDelegate.AddDynamic(this, &AShipPawn::OnStartGame);
        }
        activateComponent(this->FireComponent);
        activateComponent(this->RepairComponent);
        activateComponent(this->SkillComponent);
    }
    else
    {
        if (!this->IsLocallyControlled())
        {
            if (!ensure(this->TargetComponent != nullptr)) return;
            this->TargetComponent->SetChildActorClass(this->TargetClass);
            this->TargetComponent->CreateChildActor();

            if (!ensure(this->DriverMeshComponent != nullptr)) return;
            this->DriverMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        }
        else
        {
            UHub_SpacelGameInstance* spacelGameInstance{ Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
            spacelGameInstance->OnTargetPlayerDelegate.AddDynamic(this, &AShipPawn::OnTargetPlayer);
            spacelGameInstance->OnUnTargetPlayerDelegate.AddDynamic(this, &AShipPawn::OnUnTargetPlayer);

            this->WidgetTargetComponent = Cast<UWidgetInteractionComponent>(this->GetComponentByClass(UWidgetInteractionComponent::StaticClass()));

            // add speed line component
            if (ULocalPlayerActionComponent* localPlayerActionComponent = NewObject<ULocalPlayerActionComponent>(this, "LocalPlayerAction_00"))
            {
                localPlayerActionComponent->RegisterComponent();
            }

            // remove collision for local player (for disable hit with cursor for target)
            setCollisionProfile("NoOverlapTeam");
            this->DriverMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        }
    }
}

void AShipPawn::RPCClientChangeStateEscapeMode_Implementation(ECountDown _newState)
{
    if (this->IsLocallyControlled())
    {
        this->OnStateEspaceModeChangeDelegate.Broadcast(_newState);
    }
}

bool AShipPawn::isTargetPlayer() const
{
    if (!ensure(this->FireComponent != nullptr)) return false;
    return this->FireComponent->m_target != nullptr;
}

void AShipPawn::launchMissile()
{
    if (!ensure(this->FireComponent != nullptr)) return;
    if (!ensure(this->ModuleComponent != nullptr)) return;
    if (!ensure(this->ModuleComponent->MissileMeshComponent != nullptr)) return;
    this->FireComponent->launchMissile(this->ModuleComponent->MissileMeshComponent->GetComponentTransform());
}

void AShipPawn::OnTargetPlayer(AActor* _target)
{
    rpcTargetCall(_target, std::bind(&AShipPawn::RPCServerTargetPlayer, this, std::placeholders::_1));
}

void AShipPawn::RPCServerTargetPlayer_Implementation(int32 _playerId)
{
    if (!ensure(this->FireComponent != nullptr)) return;

    if (AGameStateBase* gameState = UGameplayStatics::GetGameState(this->GetWorld()))
    {
        for (APlayerState const* playerState : gameState->PlayerArray)
        {
            if (playerState)
            {
                if (playerState->PlayerId == _playerId)
                {
                    AActor* act = playerState->GetPawn();
                    this->FireComponent->m_target = act;
                    UE_LOG(LogTemp, Warning, TEXT("Target actor"));
                    break;
                }
            }
        }
    }
}

void AShipPawn::OnUnTargetPlayer(class AActor* _target)
{
    rpcTargetCall(_target, std::bind(&AShipPawn::RPCServerUnTargetPlayer, this, std::placeholders::_1));
}

void AShipPawn::RPCServerUnTargetPlayer_Implementation(int32 _playerId)
{
    if (!ensure(this->FireComponent != nullptr)) return;

    if (AGameStateBase* gameState = UGameplayStatics::GetGameState(this->GetWorld()))
    {
        for (APlayerState const* playerState : gameState->PlayerArray)
        {
            AActor* act { playerState->GetPawn() };
            if (playerState != nullptr
                && playerState->PlayerId == _playerId
                && act != nullptr
                && this->FireComponent->m_target != nullptr
                && act->GetUniqueID() == this->FireComponent->m_target->GetUniqueID())
            {
                this->FireComponent->m_target = nullptr;
                break;
            }
        }
    }
}

void AShipPawn::rpcTargetCall(class AActor* _target, std::function<void(int32)> _rpc)
{
    _rpc(AShipPawn::getPlayerIdFromTarget(_target));
}

int32 AShipPawn::getPlayerIdFromTarget(AActor* _target)
{
    if (_target != nullptr)
    {
        if (AShipPawn const* pawnOwner = Cast<AShipPawn>(_target->GetParentActor()))
        {
            if (ASpacelPlayerState* playerState = pawnOwner->GetPlayerState<ASpacelPlayerState>())
            {
                return playerState->PlayerId;
            }
        }
    }

    ensure(false);
    return {};
}

// Called every frame
void AShipPawn::Tick(float _deltaTime)
{
	Super::Tick(_deltaTime);

    if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        // move ship
        serverMove(_deltaTime);
    }
}

void AShipPawn::lookAt(FVector const& _loc, FVector const& _dir, FVector const& _hitLoc)
{
    FVector farpoint = _loc + _dir * 3000;
    FVector bigFar = _loc + _dir * 100000;
    this->TargetLocation = _hitLoc.IsNearlyZero() ? bigFar : _hitLoc;

    FRotator rotation = SimplyMath::MyLookRotation(bigFar, this->GetActorUpVector(), this->GetActorLocation());
    this->SetActorRotation(rotation);
}

void AShipPawn::serverMove(float _deltaTime)
{
    if (!ensure(this->DriverMeshComponent != nullptr)) return;
    if (!ensure(this->PlayerDataAsset != nullptr)) return;
    if (!ensure(this->ModuleComponent != nullptr)) return;
    if (!ensure(this->ModuleComponent->SupportMeshComponent != nullptr)) return;

    float coefEscape { m_isEscape ? this->PlayerDataAsset->EscapeModeCoef : 1.0f };

    // roll rotation

    FRotator rotation = this->GetActorRotation();
    rotation.Add(0.0f, 0.0f, this->PercentFlightAttitude * coefEscape * this->PlayerDataAsset->MaxFlightAttitudeSpeed);
    this->SetActorRotation(rotation);

    // linear

    FVector const& linearVelocity = this->DriverMeshComponent->GetPhysicsLinearVelocity(NAME_None);
    // 9, default support size
    float coefSpeed = FMath::Max((this->ModuleComponent->SupportMeshComponent->GetInstanceCount() / 9.0f) * coefEscape, 0.2f);

    FVector newVelocity = this->DriverMeshComponent->GetForwardVector() * this->PlayerDataAsset->MaxForwardSpeed * this->RU_PercentSpeed * coefSpeed;
    newVelocity += this->DriverMeshComponent->GetRightVector() * this->PlayerDataAsset->MaxHorizontalSpeed * this->PercentHorizontalStraf * coefSpeed;
    newVelocity += this->DriverMeshComponent->GetUpVector() * this->PlayerDataAsset->MaxVerticalSpeed * this->PercentVerticalStraf * coefSpeed;
    newVelocity = FMath::Lerp(linearVelocity, newVelocity, 0.9f);

    this->DriverMeshComponent->SetPhysicsLinearVelocity(newVelocity);
}

void AShipPawn::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(this->GetPlayerState());
    if (spacelPlayerState != nullptr)
    {
        FString teamName{ spacelPlayerState->Team };
        if (teamName.Len() > 0)
        {
            // TO DO : Change color for teammate and ennemy team and our pawn
        }

        setCollisionProfile(teamName);
    }
}

void AShipPawn::BuildDefaultShip()
{
#if WITH_EDITOR
    this->ModuleComponent->OnStartGame();
#endif
}

void AShipPawn::setFire(bool _on)
{
    if (!ensure(this->FireComponent != nullptr)) return;
    this->FireComponent->m_isFire = _on;
}

void AShipPawn::kill()
{
    if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        UHub_SpacelGameInstance* spacelGameInstance{ Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
        spacelGameInstance->OnUnTargetPlayerDelegate.Broadcast(this->TargetComponent->GetChildActor());

        auto lb = [](auto* _obj)
        {
            if (_obj != nullptr)
            {
                _obj->kill();
            }
        };

        lb(this->GetController<AGamePlayerController>());
        lb(this->ModuleComponent);
        lb(this->RepairComponent);

        setFire(false);

        this->GetWorldTimerManager().ClearAllTimersForObject(this);

        // temp respawn
        FTimerHandle handle;
        this->GetWorldTimerManager().SetTimer(handle, this, &AShipPawn::Restarted, 1.0f, false, 10.0f);

        // disable collision
        setCollisionProfile("NoCollision");

        m_isKilled = true;
    }
}

void AShipPawn::Restarted()
{
    auto lb = [](auto* _obj)
    {
        if (_obj != nullptr)
        {
            _obj->restarted();
        }
    };

    lb(this->GetController<AGamePlayerController>());
    lb(this->ModuleComponent);

    // enable collision
    setCollisionProfile(this->Team.ToString());

    m_isKilled = false;
}

void AShipPawn::setCollisionProfile(FString _team)
{
    if (!ensure(this->DriverMeshComponent != nullptr)) return;
    
    _team = _team.Replace(TEXT(" "), TEXT(""));
    this->DriverMeshComponent->SetCollisionProfileName(*_team);

    this->ModuleComponent->setCollisionProfile(_team);
}

void AShipPawn::setIsInFog(bool _isIn)
{
    this->RU_IsInFog = _isIn;
}

void AShipPawn::OnRep_IsInFog()
{
    if (this->TargetComponent != nullptr)
    {
        if (ATargetActor* targetActor = Cast<ATargetActor>(this->TargetComponent->GetChildActor()))
        {
            targetActor->showTarget(!this->RU_IsInFog);

            if (this->RU_IsInFog)
            {
                UHub_SpacelGameInstance* spacelGameInstance{ Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
                spacelGameInstance->OnUnTargetPlayerDelegate.Broadcast(targetActor);
            }
        }
    }
}

void AShipPawn::OnRep_Matiere()
{
    if (this->OnEndUpdateMatiereDelegate.IsBound())
    {
        this->OnEndUpdateMatiereDelegate.Broadcast(this->RU_Matiere);
    }
}

void AShipPawn::OnRep_PercentSpeed()
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

void AShipPawn::hit(FString const& _team, class UPrimitiveComponent* _comp, int32 _index)
{
    UCustomCollisionComponent* customCollisionComponent { Cast<UCustomCollisionComponent>(this->GetComponentByClass(UCustomCollisionComponent::StaticClass())) };
    if (customCollisionComponent != nullptr)
    {
        customCollisionComponent->hit(_team, _comp, _index);
    }
}

void AShipPawn::setLocationExhaustFx(TArray<FVector> const& _loc)
{
    if (this->ExhaustFxComponent == nullptr)
    {
        this->ExhaustFxComponent = Cast<UNiagaraComponent>(this->GetComponentByClass(UNiagaraComponent::StaticClass()));
    }

    if (this->ExhaustFxComponent != nullptr)
    {
        this->ExhaustFxComponent->SetNiagaraVariableInt("User.NbExhaust", _loc.Num());
        for (int32 i = 0; i < _loc.Num(); ++i)
        {
            FString name = "User.Location" + FString::FromInt(i);
            this->ExhaustFxComponent->SetNiagaraVariableVec3(name, _loc[i]);
        }
    }
}

void AShipPawn::RPCClientPlayCameraShake_Implementation()
{
    if (this->IsLocallyControlled())
    {
        if (APlayerController* playerController = this->GetController<APlayerController>())
        {
            FVector const& camLoc = this->CameraComponent->GetComponentLocation();
            playerController->ClientPlayCameraShake(this->CameraShakeClass, 1.0f, ECameraAnimPlaySpace::UserDefined, (this->GetActorLocation() - camLoc).Rotation());
        }
    }
}

float AShipPawn::getPercentProtection() const
{
    if(this->ModuleComponent == nullptr) return 0.0f;

    return this->ModuleComponent->getPercentProtection();
}

float AShipPawn::getPercentSupport() const
{
    if (this->ModuleComponent == nullptr) return 0.0f;

    return this->ModuleComponent->getPercentSupport();
}

void AShipPawn::useSkill(float _slot)
{
    if (this->SkillComponent != nullptr)
    {
        this->SkillComponent->useSkill(_slot);
    }
}

void AShipPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AShipPawn, RU_IsInFog);
    DOREPLIFETIME(AShipPawn, RU_Matiere);
    DOREPLIFETIME(AShipPawn, RU_PercentSpeed);
}

