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
#include "Mesh/XmlInstancedStaticMeshComponent.h"
#include "Mesh/SpacelInstancedMeshComponent.h"
#include "Mesh/AnimatedSpacelMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "DataAsset/StaticMeshDataAsset.h"
#include "DataAsset/SetupAttributeDataAsset.h"
#include "DataAsset/PlayerDataAsset.h"
#include "DataAsset/TeamColorDataAsset.h"
#include "DataAsset/SkillDataAsset.h"
#include "DataAsset/UniqueSkillDataAsset.h"
#include "DataAsset/MissionDataAsset.h"
#include "DataAsset/EditorHackDataAsset.h"
#include "Player/SpacelPlayerState.h"
#include "Player/TargetActor.h"
#include "Player/FireComponent.h"
#include "Player/ModuleComponent.h"
#include "Player/CustomCollisionComponent.h"
#include "Player/GamePlayerController.h"
#include "Player/RepairComponent.h"
#include "Player/LocalPlayerActionComponent.h"
#include "Player/PlayerNameActor.h"
#include "Player/MetricComponent.h"
#include "GameState/SpacelGameState.h"
#include "Hub_SpacelGameInstance.h"
#include "Util/Tag.h"
#include "Util/SimplyMath.h"
#include "TimerManager.h"
#include "Gameplay/SkillComponent.h"
#include "Gameplay/Skill/PostProcessInvisible.h"
#include "Gameplay/Skill/HealPackBullet.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

void AShipPawn::OnChangeState(EGameState _state)
{
    if (_state == EGameState::LockPrepare)
    {
        if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
        {
            this->StartTransform = GetActorTransform();

            // add custom collision component
            if (UCustomCollisionComponent* customCollisionComponent = NewObject<UCustomCollisionComponent>(this, "CustomCollision_00"))
            {
                customCollisionComponent->RegisterComponent();
            }

            // TO DO : we can remove client start game rpc, and add it in netmulticaststartgame
            RPCClientStartGame(this->Team);
            RPCNetMulticastStartGame(this->Team);
        }

        if (this->SkillComponent != nullptr)
        {
            this->SkillComponent->setupSkill();
        }

        if (this->IsLocallyControlled())
        {
            if (!ensure(this->SpringArmComponent != nullptr)) return;
            this->SpringArmComponent->SetRelativeLocation(FVector::ZeroVector);
            this->SpringArmComponent->SetRelativeRotation(m_defaultSprintArmRotator);
        }
    }
    else if (_state == EGameState::InGame)
    {
        if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
        {
            addEffect(EEffect::StartGame);
            FTimerDelegate timerCallback;
            timerCallback.BindLambda([&]() { removeEffect(EEffect::StartGame); });

            FTimerHandle handle;
            this->GetWorldTimerManager().SetTimer(handle, timerCallback, 3.0f, false);

#if WITH_EDITOR
            if (this->HackDataAsset != nullptr)
            {
                if (this->HackDataAsset->UseHack)
                {
                    addMatiere(this->HackDataAsset->MatiereAtBeginning, EMatiereOrigin::Farm);
                }
            }
#endif
        }

        if (this->SkillComponent != nullptr)
        {
            this->SkillComponent->SetupSpecialSkill();
        }
    }
    else if (_state == EGameState::EndGame)
    {
        if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
        {
            setFire(false);
            this->GetWorldTimerManager().ClearAllTimersForObject(this);
            this->DriverMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
            this->DriverMeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        }
        m_endGame = true;
    }
}

void AShipPawn::RPCNetMulticastStartGame_Implementation(FName const& _team)
{
    if(this->GetNetMode() == ENetMode::NM_DedicatedServer || this->IsLocallyControlled()) return;

    // find local player
    FString localTeam {};
    if (APlayerController const* playerController = UGameplayStatics::GetPlayerController(this->GetWorld(), 0))
    {
        if (ASpacelPlayerState const* playerState = playerController->GetPlayerState<ASpacelPlayerState>())
        {
            localTeam = playerState->R_Team;
        }
    }

    // check if we are same team as local player
    if (!_team.IsEqual(*localTeam))
    {
        if (!ensure(this->TargetComponent != nullptr)) return;
        this->TargetComponent->SetChildActorClass(this->TargetClass);
        this->TargetComponent->CreateChildActor();
    }

    if (!ensure(this->PlayerNameComponent != nullptr)) return;
    if (APlayerNameActor* playerNameActor = Cast<APlayerNameActor>(this->PlayerNameComponent->GetChildActor()))
    {
        if (this->TeamColorDataAsset != nullptr)
        {
            if (APlayerState const* playerState = this->GetPlayerState())
            {
                playerNameActor->setPlayerName(playerState->GetPlayerName(), this->TeamColorDataAsset->GetColor<FSlateColor>(_team.ToString()));
            }
        }
    }
}

void AShipPawn::RPCClientStartGame_Implementation(FName const& _team)
{
    if (this->IsLocallyControlled())
    {
        this->OnLocalTeamUpdateDelegate.Broadcast(_team.ToString());
    }

    this->Team = _team;

    BP_OnStartGame();
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
            spacelGameState->OnChangeStateDelegate.AddDynamic(this, &AShipPawn::OnChangeState);
            spacelGameState->OnPlayerEnterFogDelegate.AddDynamic(this, &AShipPawn::OnPlayerEnterFog);
            spacelGameState->OnEndMissionDelegate.AddDynamic(this, &AShipPawn::OnEndMission);
        }
        activateComponent(this->FireComponent);
        activateComponent(this->RepairComponent);
        activateComponent(this->SkillComponent);

        FTimerHandle handle;
        this->GetWorldTimerManager().SetTimer(handle, this, &AShipPawn::LinkPawn, 1.0f, false);

        if (this->PlayerDataAsset != nullptr)
        {
            // for give matiere on first farm
            m_nbAsteroideFarm = this->PlayerDataAsset->NbAsteroideForMatiere;
        }
    }
    else
    {
        if (!this->IsLocallyControlled())
        {
            if (!ensure(this->DriverMeshComponent != nullptr)) return;
            this->DriverMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

            if (!ensure(this->PlayerNameComponent != nullptr)) return;
            this->PlayerNameComponent->SetChildActorClass(this->PlayerNameClass);
            this->PlayerNameComponent->CreateChildActor();
        }
        else
        {
            this->WidgetTargetComponent = Cast<UWidgetInteractionComponent>(this->GetComponentByClass(UWidgetInteractionComponent::StaticClass()));

            // add speed line component
            if (ULocalPlayerActionComponent* localPlayerActionComponent = NewObject<ULocalPlayerActionComponent>(this, "LocalPlayerAction_00"))
            {
                localPlayerActionComponent->RegisterComponent();
            }

            // remove collision for local player (for disable hit with cursor for target)
            setCollisionProfile("NoOverlapTeam");
            this->DriverMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

            activateComponent(this->SkillComponent);

            ASpacelGameState* spacelGameState{ Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())) };
            if (spacelGameState != nullptr)
            {
                spacelGameState->OnChangeStateDelegate.AddDynamic(this, &AShipPawn::OnChangeState);
            }
            UHub_SpacelGameInstance* spacelGameInstance = Cast<UHub_SpacelGameInstance>(this->GetGameInstance());
            if (!ensure(spacelGameInstance != nullptr)) return;
            RPCServerSetPlayerName(spacelGameInstance->CustomPlayerName);

            if (!ensure(this->SpringArmComponent != nullptr)) return;
            m_defaultSprintArmRotator = this->SpringArmComponent->GetRelativeRotation();

            this->SpringArmComponent->SetRelativeLocation(FVector(-10.0f, 40.0f, -60.0f));
            FVector rot(15.0f, -25.0f, -15.0f);
            this->SpringArmComponent->SetRelativeRotation(rot.ToOrientationRotator().Quaternion());
        }
    }
}

void AShipPawn::OnEndMission(EMission _type, bool _succeed, FName _succeedForTeam)
{
    if (_type == EMission::HoldGold)
    {
        if (hasEffect(EEffect::Gold))
        {
            if (this->MissionDataAsset != nullptr)
            {
                FMission const& mission = this->MissionDataAsset->getMission(EMission::HoldGold);
                boostPassive(_type, mission.RewardValue);
            }
            removeEffect(EEffect::Gold);
        }
    }
}

void AShipPawn::RPCServerSetPlayerName_Implementation(FString const& _playerName)
{
    if (ASpacelPlayerState* spacelPlayerState = GetPlayerState<ASpacelPlayerState>())
    {
        spacelPlayerState->SetPlayerName(_playerName);
    }
}

void AShipPawn::LinkPawn()
{
    if (AGamePlayerController* gamePlayerController = this->GetController<AGamePlayerController>())
    {
        gamePlayerController->LinkPawn = this;
    }
    else
    {
        FTimerHandle handle;
        this->GetWorldTimerManager().SetTimer(handle, this, &AShipPawn::LinkPawn, 1.0f, false);
    }
}

void AShipPawn::fireShotgun()
{
    if (!ensure(this->FireComponent != nullptr)) return;
    this->FireComponent->fireShotgun();
}

void AShipPawn::emp(uint32 _duration, FName const& _team, int32 _playerId)
{
    m_lastTeamEmp = _team;
    m_lastPlayerIdEmp = _playerId;
    addEffect(EEffect::Emp);
    FTimerHandle handle;
    this->GetWorldTimerManager().SetTimer(handle, this, &AShipPawn::CleanEmp, _duration, false);
}

void AShipPawn::CleanEmp()
{
    if (AGamePlayerController* playerController = this->GetController<AGamePlayerController>())
    {
        removeEffect(EEffect::Emp);
    }
}

// Called every frame
void AShipPawn::Tick(float _deltaTime)
{
	Super::Tick(_deltaTime);

    if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        if (!hasEffect(EEffect::Killed) && !m_endGame)
        {
            // move ship
            moveShip(_deltaTime);
            updateAssist(_deltaTime);
        }
    }
    else if(!m_endGame)
    {
        computeSoundData();
    }
}

void AShipPawn::computeSoundData()
{
    int percentSpeed = (int)FMath::Clamp((FMath::Abs(RU_PercentSpeed) + R_OverDrive) * 100, 0.0f, 100.0f);
    if (hasEffect(EEffect::MetaFormAttack) || hasEffect(EEffect::MetaFormProtection) || hasEffect(EEffect::MetaFormSupport) || hasEffect(EEffect::EscapeMode))
    {
        // override speed max
        percentSpeed = 101 * FMath::Abs(RU_PercentSpeed);
    }
    else if (hasEffect(EEffect::Missile))
    {
        percentSpeed = 0.0f;
    }

    bool playStart { false };
    if (FMath::IsNearlyEqual(RU_PercentSpeed, 0.0f) && m_lastDirection != 0)
    {
        m_lastDirection = 0;
    }
    else if (RU_PercentSpeed > 0 && m_lastDirection != 1)
    {
        m_lastDirection = 1;
        playStart = true;
    }
    else if (RU_PercentSpeed < 0 && m_lastDirection != -1)
    {
        m_lastDirection = -1;
    }

    BP_FxSpeedSound(percentSpeed, playStart);
}

void AShipPawn::emergencyRedCube()
{
    // enable emergency skill
    if (this->SkillComponent != nullptr)
    {
        this->SkillComponent->emergencyRedCube();
        this->SkillComponent->RPCClientEmergencyRedCube();
    }
}

void AShipPawn::onEmergencyCountDownEnd()
{
    // remove skill
    if (this->SkillComponent != nullptr)
    {
        this->SkillComponent->emergencyRedCubeRemove();
        this->SkillComponent->RPCClientEmergencyRedCubeRemove();
    }
}

void AShipPawn::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    ASpacelPlayerState* spacelPlayerState = Cast<ASpacelPlayerState>(this->GetPlayerState());
    if (spacelPlayerState != nullptr)
    {
        FString teamName{ spacelPlayerState->R_Team };
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
    this->ModuleComponent->OnChangeState(EGameState::InGame);
#endif
}

void AShipPawn::buildLobbyShip(ESkill _skillId, ESkillType _type)
{
    this->ModuleComponent->buildLobbyShip(_skillId, _type);
}

void AShipPawn::setFire(bool _on)
{
    if (!ensure(this->FireComponent != nullptr)) return;
    this->FireComponent->m_isFire = _on;
}

void AShipPawn::kill(int32 _playerId)
{
    if (this->GetNetMode() == ENetMode::NM_DedicatedServer)
    {
        // dispatch assist
        riseAssist(_playerId);

        auto lb = [](auto* _obj)
        {
            if (_obj != nullptr)
            {
                _obj->kill();
            }
        };

        lb(this->GetController<AGamePlayerController>());
        lb(this->ModuleComponent);

        // remove skill emergency if exist
        if (this->SkillComponent != nullptr)
        {
            this->SkillComponent->emergencyRedCubeRemove();
            this->SkillComponent->RPCClientEmergencyRedCubeRemove();
        }

        setFire(false);

        // fing all fog actor
        if (auto* world = this->GetWorld())
        {
            TArray<AActor*> fogs;
            UGameplayStatics::GetAllActorsWithTag(world, Tags::Fog, fogs);

            if (fogs.Num() > 0)
            {
                int id { FMath::RandRange(0, fogs.Num() - 1) };

                if (id < fogs.Num())
                {
                    // move pawn into fog
                    this->SetActorLocationAndRotation(fogs[id]->GetActorLocation(), fogs[id]->GetActorRotation(), false, nullptr, ETeleportType::ResetPhysics);
                    this->DriverMeshComponent->SetWorldLocationAndRotationNoPhysics(fogs[id]->GetActorLocation(), fogs[id]->GetActorRotation());
                }
            }
        }

        this->GetWorldTimerManager().ClearAllTimersForObject(this);

        // temp respawn
        FTimerHandle handle;
        this->GetWorldTimerManager().SetTimer(handle, this, &AShipPawn::Restarted, 1.0f, false, 10.0f);

        // disable collision
        if (this->DriverMeshComponent != nullptr)
        {
            this->DriverMeshComponent->SetCollisionProfileName("Killed");
        }

        addEffect(EEffect::Killed);
        removeEffect(EEffect::Emp);
        removeEffect(EEffect::BackToGame);

        // reset physic
        this->DriverMeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        this->DriverMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);

        this->RU_Matiere = 0;
        OnRep_Matiere();
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
    if (this->DriverMeshComponent != nullptr)
    {
        auto const team = this->Team.ToString().Replace(TEXT(" "), TEXT(""));
        this->DriverMeshComponent->SetCollisionProfileName(*team);
    }

    removeEffect(EEffect::Killed);
    addEffect(EEffect::Respawned);

    FTimerDelegate timerCallback;
    timerCallback.BindLambda([&]() { removeEffect(EEffect::Respawned); });

    FTimerHandle handle;
    this->GetWorldTimerManager().SetTimer(handle, timerCallback, 1.0f, false);

    // clean assist
    m_assistPlayer.Empty();
}

void AShipPawn::OnPlayerEnterFog(int32 _playerId, bool _enter)
{
    if (APlayerState* playerState = this->GetPlayerState())
    {
        if (playerState->PlayerId == _playerId)
        {
            RPCNetMulticastEnterFog(_playerId, _enter);
        }
    }
}

void AShipPawn::RPCNetMulticastEnterFog_Implementation(int32 _playerId, bool _enter)
{
    if (this->TargetComponent != nullptr)
    {
        if (ATargetActor* targetActor = Cast<ATargetActor>(this->TargetComponent->GetChildActor()))
        {
            targetActor->showTarget(!_enter);

            if (!ensure(this->PlayerNameComponent != nullptr)) return;
            if (APlayerNameActor* playerNameActor = Cast<APlayerNameActor>(this->PlayerNameComponent->GetChildActor()))
            {
                playerNameActor->show(!_enter);
            }
        }
    }
}

void AShipPawn::OnRep_Matiere()
{
    // only on local player
    if (this->OnEndUpdateMatiereDelegate.IsBound())
    {
        int32 delta = this->RU_Matiere - m_lastMatiere;
        FString str = FString::FromInt(delta);
        if (delta > 0)
        {
            FString signe{ "+" };
            str = signe + str;
        }

        BP_FxAddMatiere(delta);

        m_lastMatiere = this->RU_Matiere;
        this->OnEndUpdateMatiereDelegate.Broadcast(this->RU_Matiere, str);
    }
}

void AShipPawn::hit(FString const& _team, int32 _playerId, class UPrimitiveComponent* _comp, int32 _index, FVector const& _otherLocation, AActor* _otherActor)
{
    UCustomCollisionComponent* customCollisionComponent{ Cast<UCustomCollisionComponent>(this->GetComponentByClass(UCustomCollisionComponent::StaticClass())) };
    if (customCollisionComponent != nullptr)
    {
        customCollisionComponent->hit(_team, _playerId, _comp, _index, _otherLocation, _otherActor);
    }

    bool bfind { false };
    for (auto assist : m_assistPlayer)
    {
        if (assist.m_playerId == _playerId)
        {
            assist.m_timer = 30.0f;
            bfind = true;
        }
    }

    if (!bfind)
    {
        m_assistPlayer.Add({ _playerId });
    }
}

void AShipPawn::setLocationExhaustFx(TArray<FVector_NetQuantize> const& _loc)
{
    TArray<FVector_NetQuantize> cpy = _loc;

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
        if (exhaust != nullptr)
        {
            if (cpy.Num() != 0)
            {
                exhaust->SetNiagaraVariableFloat("User.Velocity", this->RU_PercentSpeed);
                exhaust->SetRelativeLocation(cpy[0]);
                cpy.RemoveAt(0);
                exhaust->SetActive(true, true);
            }
            else if (exhaust->IsActive())
            {
                exhaust->SetActive(false, true);
            }
        }
    }
}

void AShipPawn::RPCClientPlayCameraShake_Implementation(EImpactType _type)
{
    if (this->IsLocallyControlled())
    {
        if (APlayerController* playerController = this->GetController<APlayerController>())
        {
            FVector const& camLoc = this->CameraComponent->GetComponentLocation();
            if (_type == EImpactType::Obstacle)
            {
                playerController->ClientPlayCameraShake(this->CameraShakeObstacleClass, 1.0f, ECameraAnimPlaySpace::UserDefined, (this->GetActorLocation() - camLoc).Rotation());
            }
            else if (_type == EImpactType::Hit)
            {
                playerController->ClientPlayCameraShake(this->CameraShakeHitClass, 1.0f, ECameraAnimPlaySpace::UserDefined, (this->GetActorLocation() - camLoc).Rotation());
            }
        }

        //BP_HitIndicator();
    }
}

float AShipPawn::getPercentProtection() const
{
    if (this->ProtectionComponent == nullptr) return 0.0f;

    return this->ProtectionComponent->GetNum() / this->ProtectionComponent->GetMax();
}

float AShipPawn::getPercentSupport() const
{
    if (this->SupportComponent == nullptr) return 0.0f;

    return this->SupportComponent->GetNum() / this->SupportComponent->GetMax();
}

void AShipPawn::boostPassive(EMission _type, int32 _rewardValue)
{
    switch (_type)
    {
        case EMission::Pirate:
            addEffect(EEffect::PassiveFireRate);
            m_bonusFireRate = _rewardValue;
        break;

        case EMission::Comet:
            addEffect(EEffect::PassiveCountDown);
            R_BonusCountDown = _rewardValue;
        break;

        case EMission::HoldGold:
            addEffect(EEffect::PassiveSpeed);
            m_bonusSpeed = _rewardValue;
        break;
    }
}

bool AShipPawn::canTank(TArray<FHitResult> const& _hits)
{
    auto lb_addScore = [&](int32 _val)
    {
        if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())))
        {
            if(ASpacelPlayerState const* playerState = this->GetPlayerState<ASpacelPlayerState>())
            spacelGameState->AddScore(this->Team.ToString(), playerState->PlayerId, EScoreType::Tank, _val);
        }
    };

    if (hasEffect(EEffect::MetaFormProtection))
    {
        return true;
    }
    else if (hasEffect(EEffect::Respawned) || hasEffect(EEffect::StartGame))
    {
        return true;
    }

    if (hasEffect(EEffect::Farmer))
    {
        bool ret = true;
        int32 add = 0;
        for (auto hit : _hits)
        {
            if (hit.Actor.IsValid() && hit.Actor->Tags.Contains(Tags::Asteroide))
            {
                ++add;
            }
            else
            {
                ret = false;
            }
        }

        if (add != 0)
        {
            addMatiere(add, EMatiereOrigin::Farm);
        }
        return ret;
    }

    return false;
}

void AShipPawn::RPCNetMulticastEnterHidding_Implementation(int32 _playerId, bool _enter)
{
    if(this->RootComponent == nullptr) return;
    if(this->IsLocallyControlled() || this->GetNetMode() == ENetMode::NM_DedicatedServer) return;

    if (_enter)
    {
        this->RootComponent->SetVisibility(false, true);
    }
    else
    {
        this->RootComponent->SetVisibility(true, true);
        if (this->BaseShipMeshComponent != nullptr)
        {
            this->BaseShipMeshComponent->SetVisibility(false);
        }

        TArray<UActorComponent*> components = this->GetComponentsByTag(USceneComponent::StaticClass(), Tags::Arrow);
        for (auto component : components)
        {
            if (USceneComponent* sceneComponent = Cast<USceneComponent>(component))
            {
                sceneComponent->SetVisibility(false);
            }
        }
    }
}

void AShipPawn::RPCNetMulticastAddEffect_Implementation(EEffect _effect)
{
    BP_FxGlobalAddEffect(_effect);
    playFash(_effect);
}

void AShipPawn::RPCNetMulticastRemoveEffect_Implementation(EEffect _effect)
{
    BP_FxGlobalRemoveEffect(_effect);
    playFash(_effect);
}

void AShipPawn::playFash(EEffect _effect)
{
    switch (_effect)
    {
    case EEffect::EscapeMode:
    case EEffect::MetaFormAttack:
    case EEffect::MetaFormProtection:
    case EEffect::MetaFormSupport:
    case EEffect::Missile:
    case EEffect::BulletStun:
    case EEffect::Farmer:
    case EEffect::HealZone:
        BP_FxChangeForm();
        break;
    }
}

void AShipPawn::RPCClientAddEffect_Implementation(EEffect _effect)
{
    OnAddEffectClientDelegate.Broadcast(_effect);
    BP_FxAddEffect(_effect);

    if (_effect == EEffect::BackToGame)
    {
        // TO DO Text system
        OnSendInfoPlayerDelegate.Broadcast("Didn't you seriously want to quit?");
    }
    else if (_effect == EEffect::MetaFormAttack)
    {
        if (this->WeaponComponent != nullptr)
        {
            this->WeaponComponent->SetScalarParameterValueOnMaterials("Edge sharp max", 0.0f);
            this->WeaponComponent->SetScalarParameterValueOnMaterials("Glow", 1200.0f);
        }
    }
    else if (_effect == EEffect::Emp)
    {
        if (this->PlayerDataAsset != nullptr)
        {
            FTransform const& transform = this->GetActorTransform();
            if (APostProcessInvisible* actor = Cast<APostProcessInvisible>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this->GetWorld(), this->PlayerDataAsset->EmpPostProcessClass, transform)))
            {
                actor->Effect = _effect;
                OnRemoveEffectDelegate.AddDynamic(actor, &APostProcessInvisible::OnRemoveEffect);
                UGameplayStatics::FinishSpawningActor(actor, transform);
                actor->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));
            }
        }
    }
    else if (_effect == EEffect::Killed)
    {
        // reset local physic
        this->DriverMeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        this->DriverMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);
    }
}

void AShipPawn::RPCClientRemoveEffect_Implementation(EEffect _effect)
{
    OnRemoveEffectDelegate.Broadcast(_effect);
    BP_FxRemoveEffect(_effect);

    if (_effect == EEffect::MetaFormAttack)
    {
        if (this->WeaponComponent != nullptr)
        {
            this->WeaponComponent->SetScalarParameterValueOnMaterials("Edge sharp max", 550.0f);
            this->WeaponComponent->SetScalarParameterValueOnMaterials("Glow", 800.0f);
        }
    }
}

void AShipPawn::behaviourAddEffect(EEffect _type)
{
    if (_type == EEffect::Farmer)
    {
        if (this->ModuleComponent != nullptr)
        {
            this->ModuleComponent->activeMetaForm(_type);
        }
    }
    else if (_type == EEffect::Fog || _type == EEffect::Killed)
    {
        if (APlayerState* playerState = this->GetPlayerState())
        {
            if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())))
            {
                spacelGameState->OnPlayerEnterFogDelegate.Broadcast(playerState->PlayerId, true);
            }
        }

        if (_type == EEffect::Killed)
        {
            this->RU_LeftTrail = false;
            this->RU_RightTrail = false;
        }
    }
    else if (_type == EEffect::Emp)
    {
        if (this->FireComponent != nullptr)
        {
            this->FireComponent->m_isFire = false;
        }
    }
    else if (_type == EEffect::BackToGame)
    {
        if (this->FireComponent != nullptr)
        {
            this->FireComponent->m_isFire = false;
        }

        FTimerHandle handle;
        this->GetWorldTimerManager().SetTimer(handle, this, &AShipPawn::BackToGame, 1.0f, false);
    }
    else if (_type == EEffect::MetaFormSupport)
    {
        if (APlayerState* playerState = this->GetPlayerState())
        {
            RPCNetMulticastEnterHidding(playerState->PlayerId, true);

            if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())))
            {
                spacelGameState->OnPlayerEnterFogDelegate.Broadcast(playerState->PlayerId, true);
            }
        }

        if (this->ModuleComponent != nullptr)
        {
            this->ModuleComponent->activeMetaForm(_type);
        }
    }
    else if (_type == EEffect::MetaFormProtection)
    {
        if (this->ModuleComponent != nullptr)
        {
            this->ModuleComponent->activeMetaForm(_type);
        }
    }
    else if (_type == EEffect::MetaFormAttack)
    {
        if (this->ModuleComponent != nullptr)
        {
            this->ModuleComponent->activeMetaForm(_type);
        }
    }
    else if (_type == EEffect::StartGame)
    {
        if (this->PlayerDataAsset != nullptr)
        {
            this->R_OverDrive = this->PlayerDataAsset->CoefOverDriveValue;
        }
    }
    else if (_type == EEffect::Gold)
    {
        RPCNetMultiCastFxGold(true);
    }
    else if (_type == EEffect::EscapeMode)
    {
        if (this->ModuleComponent != nullptr)
        {
            this->ModuleComponent->activeMetaForm(_type);
        }
    }
    else if (_type == EEffect::HealZone)
    {
        if (this->ModuleComponent != nullptr)
        {
            this->ModuleComponent->activeMetaForm(_type);
        }
    }
    else if (_type == EEffect::Missile)
    {
        if (this->ModuleComponent != nullptr)
        {
            this->ModuleComponent->activeMetaForm(_type);
        }
    }
    else if (_type == EEffect::BulletStun)
    {
        if (this->ModuleComponent != nullptr)
        {
            this->ModuleComponent->activeMetaForm(_type);
        }
    }
}

void AShipPawn::RPCNetMultiCastFxGold_Implementation(bool _activate)
{
    BP_FxGold(_activate);
}

void AShipPawn::BackToGame()
{
    removeEffect(EEffect::BackToGame);
}

void AShipPawn::addEffectSuccess(EEffect _type)
{
    OnAddEffectServerDelegate.Broadcast(_type);
    behaviourAddEffect(_type);
    RPCClientAddEffect(_type);
    RPCNetMulticastAddEffect(_type);
}

void AShipPawn::removeEffectSuccess(EEffect _type)
{
    behaviourRemoveEffect(_type);
    RPCClientRemoveEffect(_type);
    RPCNetMulticastRemoveEffect(_type);
}

void AShipPawn::behaviourRemoveEffect(EEffect _type)
{
    if (_type == EEffect::Farmer)
    {
        if (this->ModuleComponent != nullptr)
        {
            this->ModuleComponent->removeMetaForm(_type);
        }
    }
    else if (_type == EEffect::Fog || _type == EEffect::Killed)
    {
        if (APlayerState* playerState = this->GetPlayerState())
        {
            if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())))
            {
                spacelGameState->OnPlayerEnterFogDelegate.Broadcast(playerState->PlayerId, false);
            }
        }
    }
    else if (_type == EEffect::Emp)
    {
        m_lastTeamEmp = FName();
        m_lastPlayerIdEmp = {};
    }
    else if (_type == EEffect::MetaFormSupport)
    {
        if (APlayerState* playerState = this->GetPlayerState())
        {
            RPCNetMulticastEnterHidding(playerState->PlayerId, false);

            if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())))
            {
                spacelGameState->OnPlayerEnterFogDelegate.Broadcast(playerState->PlayerId, false);
            }
        }

        if (this->ModuleComponent != nullptr)
        {
            this->ModuleComponent->removeMetaForm(_type);
        }
    }
    else if (_type == EEffect::MetaFormProtection)
    {
        if (this->ModuleComponent != nullptr)
        {
            this->ModuleComponent->removeMetaForm(_type);
        }
    }
    else if (_type == EEffect::MetaFormAttack)
    {
        if (this->ModuleComponent != nullptr)
        {
            this->ModuleComponent->removeMetaForm(_type);
        }
    }
    else if (_type == EEffect::EscapeMode)
    {
        if (this->ModuleComponent != nullptr)
        {
            this->ModuleComponent->removeMetaForm(_type);
        }
    }
    else if (_type == EEffect::HealZone)
    {
        if (this->ModuleComponent != nullptr)
        {
            this->ModuleComponent->removeMetaForm(_type);
        }
    }
    else if (_type == EEffect::StartGame)
    {
        if (this->PlayerDataAsset != nullptr)
        {
            this->R_OverDrive = 0;
        }
    }
    else if (_type == EEffect::Gold)
    {
        this->OnLostGoldDelegate.broadcast();
        RPCNetMultiCastFxGold(false);
    }
    else if (_type == EEffect::Missile)
    {
        if (this->ModuleComponent != nullptr)
        {
            this->ModuleComponent->removeMetaForm(_type);
        }
    }
    else if (_type == EEffect::BulletStun)
    {
        if (this->ModuleComponent != nullptr)
        {
            this->ModuleComponent->removeMetaForm(_type);
        }
    }
}

void AShipPawn::RPCClientFeedbackScore_Implementation(EScoreType _type, int16 _value)
{
    OnFeedbackScoreDelegate.Broadcast(_type, _value);
}

void AShipPawn::RPCNetMulticastFxExploseHeal_Implementation()
{
    BP_FxExploseHeal();
}

void AShipPawn::heal(uint8 _value)
{
    this->RepairComponent->heal(_value);
    RPCNetMulticastFxExploseHeal();
}

ESkillReturn AShipPawn::onRepair()
{
    if (this->RepairComponent != nullptr)
    {
        return this->RepairComponent->onRepair();
    }
    return ESkillReturn::InternError;
}

ESkillReturn AShipPawn::onSwapEmergency()
{
    if (this->ModuleComponent != nullptr)
    {
        if (this->SkillComponent != nullptr)
        {
            if (UUniqueSkillDataAsset const* skillParam = this->SkillComponent->SkillDataAsset->getSKill(ESkill::Emergency))
            {
                return this->ModuleComponent->onSwapEmergency(skillParam->Value);
            }
        }
    }
    return ESkillReturn::InternError;
}

void AShipPawn::addMatiere(int32 _val, EMatiereOrigin _type)
{
    // don't keep a negative matiere value
    this->RU_Matiere = FMath::Max(this->RU_Matiere + _val, 0);

    // in fact, _val > 0 is an useless test, because we have a type for lost matiere
    // therefor, we convert _val into uint so it's better to check if is not an negative number
    // because if in futur we have a case where we can have negative value for this type...
    if (_val > 0 && (_type == EMatiereOrigin::Farm || _type == EMatiereOrigin::Kill))
    {
        if (UMetricComponent* component = Cast<UMetricComponent>(this->GetComponentByClass(UMetricComponent::StaticClass())))
        {
            component->updateMetric<SMetricAdd, uint16>(EMetric::MatiereWin, { (uint16)_val });
        }
    }

    OnRep_Matiere();
}

void AShipPawn::farmAsteroide()
{
    if (this->PlayerDataAsset != nullptr)
    {
        m_nbAsteroideFarm++;
        if (this->PlayerDataAsset->NbAsteroideForMatiere <= m_nbAsteroideFarm)
        {
            addMatiere(1, EMatiereOrigin::Farm);
            m_nbAsteroideFarm = 0;
        }
    }
}

ESkillReturn AShipPawn::spawnHealPack()
{
    if (this->SkillComponent != nullptr)
    {
        if (UUniqueSkillDataAsset const* uniqueSkillDataAsset = this->SkillComponent->getSkill(ESkill::HealPack))
        {
            int healPackMatiere = uniqueSkillDataAsset->MatiereNeeded;
            if (this->RU_Matiere >= healPackMatiere)
            {
                addMatiere(healPackMatiere * -1, EMatiereOrigin::Lost);
                // spawn heal pack
                TArray<UActorComponent*> const& actors = this->GetComponentsByTag(USceneComponent::StaticClass(), Tags::HealPack);
                if (actors.Num() > 0 && actors[0] != nullptr)
                {
                    if (USceneComponent* comp = Cast<USceneComponent>(actors[0]))
                    {
                        FTransform tr = comp->GetComponentTransform();

                        FVector dir = UKismetMathLibrary::FindLookAtRotation(tr.GetLocation(), TargetLocation).Vector();
                        dir.Normalize();
                        tr.SetRotation(dir.ToOrientationQuat());

                        if (AHealPackBullet* healPackActor = Cast<AHealPackBullet>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), this->PlayerDataAsset->HealPackClass, tr)))
                        {
                            healPackActor->R_Team = Team;
                            if (APlayerState* playerState = GetPlayerState())
                            {
                                healPackActor->PlayerIdOwner = playerState->PlayerId;
                            }
                            healPackActor->Value = healPackMatiere;
                            healPackActor->Tags.Add(Tags::HealPack);
                            UGameplayStatics::FinishSpawningActor(healPackActor, tr);
                        }

                        return ESkillReturn::Success;
                    }
                }
            }
            else
            {
                return ESkillReturn::NoMater;
            }
        }
    }
    return ESkillReturn::InternError;
}

void AShipPawn::RPCServerSendTarget_Implementation(int32 _playerId)
{
    if(this->FireComponent == nullptr) return;

    if (AGameStateBase* gameState = UGameplayStatics::GetGameState(this->GetWorld()))
    {
        for (APlayerState const* playerState : gameState->PlayerArray)
        {
            if (playerState != nullptr && playerState->PlayerId == _playerId)
            {
                this->FireComponent->m_target = playerState->GetPawn();
                break;
            }
        }
    }
}

void AShipPawn::RPCServerResetTarget_Implementation()
{
    if (this->FireComponent == nullptr) return;
    this->FireComponent->m_target = nullptr;
}

void AShipPawn::RPCClientRepair_Implementation()
{
    BP_FxRepair();
}

void AShipPawn::updateAssist(float _deltaSeconde)
{
    m_assistPlayer.RemoveAll([&_deltaSeconde](FAssist& _obj)
        {
            _obj.m_timer -= _deltaSeconde;
            return _obj.m_timer <= 0.0f;
        });
}

void AShipPawn::riseAssist(int32 _playerId)
{
    if(_playerId == -1) return;

    // find team of player
    if (AGameStateBase* gameState = UGameplayStatics::GetGameState(this->GetWorld()))
    {
        FString refTeam {};

        for (APlayerState const* playerState : gameState->PlayerArray)
        {
            if (ASpacelPlayerState const* spacelPlayerState = Cast<ASpacelPlayerState>(playerState))
            {
                if (spacelPlayerState->PlayerId == _playerId)
                {
                    refTeam = spacelPlayerState->R_Team;
                    break;
                }
            }
        }

        if (!refTeam.IsEmpty())
        {
            for (auto data : m_assistPlayer)
            {
                for (APlayerState const* playerState : gameState->PlayerArray)
                {
                    if (data.m_playerId != _playerId)
                    {
                        if (ASpacelPlayerState const* spacelPlayerState = Cast<ASpacelPlayerState>(playerState))
                        {
                            if (spacelPlayerState->R_Team == refTeam)
                            {
                                if (AShipPawn* otherPawn = spacelPlayerState->GetPawn<AShipPawn>())
                                {
                                    if (auto metricComponent = Cast<UMetricComponent>(otherPawn->GetComponentByClass(UMetricComponent::StaticClass())))
                                    {
                                        metricComponent->updateMetric(EMetric::Assist);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void AShipPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AShipPawn, RU_Matiere);
    DOREPLIFETIME(AShipPawn, R_Effect);
}

