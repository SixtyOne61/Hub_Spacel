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
#include "DataAsset/TeamColorDataAsset.h"
#include "DataAsset/SkillDataAsset.h"
#include "DataAsset/UniqueSkillDataAsset.h"
#include "DataAsset/MissionDataAsset.h"
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
#include "Gameplay/Skill/EmpBullet.h"
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
            m_endGame = true;
        }
    }
}

void AShipPawn::RPCNetMulticastStartGame_Implementation(FName const& _team)
{
    if (this->ShieldComponent != nullptr)
    {
        if (this->TeamColorDataAsset != nullptr)
        {
            if (_team != "None")
            {
                FColor color = this->TeamColorDataAsset->GetColor<FColor>(_team.ToString());
                this->ShieldComponent->SetVectorParameterValueOnMaterials("Color", FVector{ color.ReinterpretAsLinear() });
            }
        }
    }

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

void AShipPawn::OnEndMission(EMission _type)
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

void AShipPawn::lockTarget(int32 _playerId, bool _lock)
{
    // call event on client for UI
    UHub_SpacelGameInstance* spacelGameInstance{ Cast<UHub_SpacelGameInstance>(this->GetGameInstance()) };
    spacelGameInstance->OnTargetPlayerDelegate.Broadcast(_playerId, _lock);

    RPCServerTargetPlayer(_playerId, _lock);
}

void AShipPawn::RPCServerTargetPlayer_Implementation(int32 _playerId, bool _lock)
{
    if (!ensure(this->FireComponent != nullptr)) return;

    if (_lock)
    {
        if (AGameStateBase* gameState = UGameplayStatics::GetGameState(this->GetWorld()))
        {
            for (APlayerState const* playerState : gameState->PlayerArray)
            {
                if (playerState)
                {
                    if (playerState->PlayerId == _playerId)
                    {
                        AActor* act = playerState->GetPawn();
                        if (ACommonPawn* commonPawn = Cast<ACommonPawn>(act))
                        {
                            if (APlayerState const* ourPlayerState = this->GetPlayerState<APlayerState>())
                            {
                                commonPawn->addPlayerFocusOnMe(ourPlayerState->PlayerId);
                            }
                        }

                        this->FireComponent->m_target = act;
                        addEffect(EEffect::TargetLock);
                        break;
                    }
                }
            }
        }
    }
    else
    {
        removeEffect(EEffect::TargetLock);
    }
}

void AShipPawn::launchMissile()
{
    if (!ensure(this->FireComponent != nullptr)) return;
    if (!ensure(this->ModuleComponent != nullptr)) return;
    if (!ensure(this->ModuleComponent->MissileMeshComponent != nullptr)) return;
    this->FireComponent->launchMissile(this->ModuleComponent->MissileMeshComponent->GetComponentTransform());
}

void AShipPawn::spawnKatyusha()
{
    if (!ensure(this->FireComponent != nullptr)) return;
    this->FireComponent->spawnKatyusha();
}

ESkillReturn AShipPawn::spawnEmp()
{
    if(this->SkillComponent == nullptr) return ESkillReturn::InternError;
    if (this->SkillComponent->SkillDataAsset == nullptr) return ESkillReturn::InternError;

    if (UUniqueSkillDataAsset const* skillParam = this->SkillComponent->SkillDataAsset->getSKill(ESkill::Emp))
    {
        uint32 duration = skillParam->FlatDuration;

        TArray<UActorComponent*> const& actors = this->GetComponentsByTag(USceneComponent::StaticClass(), Tags::HealPack);
        if (actors.Num() > 0 && actors[0] != nullptr)
        {
            if (USceneComponent* comp = Cast<USceneComponent>(actors[0]))
            {
                FTransform tr = comp->GetComponentTransform();

                FVector dir = UKismetMathLibrary::FindLookAtRotation(tr.GetLocation(), TargetLocation).Vector();
                dir.Normalize();
                tr.SetRotation(dir.ToOrientationQuat());

                if (AEmpBullet* empActor = Cast<AEmpBullet>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), this->PlayerDataAsset->EmpClass, tr)))
                {
                    empActor->R_Team = Team;
                    empActor->EffectDuration = duration;
                    if (APlayerState* playerState = GetPlayerState())
                    {
                        empActor->PlayerIdOwner = playerState->PlayerId;
                    }

                    empActor->Tags.Add(Tags::EmpBullet);
                    UGameplayStatics::FinishSpawningActor(empActor, tr);
                }

                return ESkillReturn::Success;
            }
        }
    }

    return ESkillReturn::Unavailable;
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
        }
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

        if (this->IsLocallyControlled())
        {
            spacelPlayerState->OnSkillLobbyChangeDelegate.AddDynamic(this, &AShipPawn::BuildShip);
        }
    }
}

void AShipPawn::BuildDefaultShip()
{
#if WITH_EDITOR
    this->ModuleComponent->OnStartGame(EGameState::InGame);
#endif
}

void AShipPawn::BuildShip()
{
    this->ModuleComponent->BuildShipLobby();
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
        auto lb = [](auto* _obj)
        {
            if (_obj != nullptr)
            {
                _obj->kill();
            }
        };

        lb(this->GetController<AGamePlayerController>());
        lb(this->ModuleComponent);

        setFire(false);

        this->GetWorldTimerManager().ClearAllTimersForObject(this);

        // temp respawn
        FTimerHandle handle;
        this->GetWorldTimerManager().SetTimer(handle, this, &AShipPawn::Restarted, 1.0f, false, 10.0f);

        this->DriverMeshComponent->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        // disable collision
        setCollisionProfile("NoCollision");

        addEffect(EEffect::Killed);
        removeEffect(EEffect::Emp);
        removeEffect(EEffect::BackToGame);
        removeAllPlayerFocusOnMe();

        // replace actor to spawn
        this->SetActorLocationAndRotation(StartTransform.GetLocation(), StartTransform.GetRotation());
        this->DriverMeshComponent->SetPhysicsLinearVelocity(FVector::ZeroVector);

        this->RU_Matiere = 0;
        OnRep_Matiere();

        if (ASpacelGameState* spacelGameState = Cast<ASpacelGameState>(UGameplayStatics::GetGameState(this->GetWorld())))
        {
            if (spacelGameState->RU_GameState == (uint8)EGameState::InGame)
            {
                spacelGameState->GoToUnlockMedium();
            }
        }
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

    removeEffect(EEffect::Killed);
    addEffect(EEffect::Respawned);

    FTimerDelegate timerCallback;
    timerCallback.BindLambda([&]() { removeEffect(EEffect::Respawned); });

    FTimerHandle handle;
    this->GetWorldTimerManager().SetTimer(handle, timerCallback, 1.0f, false);
}

void AShipPawn::OnPlayerEnterFog(int32 _playerId, bool _enter)
{
    // server side, check if it's my target
    if (_enter && this->FireComponent != nullptr && this->FireComponent->m_target != nullptr)
    {
        if (AShipPawn* pawn = Cast<AShipPawn>(this->FireComponent->m_target))
        {
            if (APlayerState* playerState = pawn->GetPlayerState())
            {
                if (playerState->PlayerId == _playerId)
                {
                    // remove target
                    removeEffect(EEffect::TargetLock);
                }
            }
        }
    }

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

void AShipPawn::hit(FString const& _team, int32 _playerId, class UPrimitiveComponent* _comp, int32 _index, FVector const& _otherLocation)
{
    UCustomCollisionComponent* customCollisionComponent{ Cast<UCustomCollisionComponent>(this->GetComponentByClass(UCustomCollisionComponent::StaticClass())) };
    if (customCollisionComponent != nullptr)
    {
        customCollisionComponent->hit(_team, _playerId, _comp, _index, _otherLocation);
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

void AShipPawn::RPCClientPlayCameraShake_Implementation()
{
    if (this->IsLocallyControlled())
    {
        if (APlayerController* playerController = this->GetController<APlayerController>())
        {
            FVector const& camLoc = this->CameraComponent->GetComponentLocation();
            playerController->ClientPlayCameraShake(this->CameraShakeClass, 1.0f, ECameraAnimPlaySpace::UserDefined, (this->GetActorLocation() - camLoc).Rotation());
        }

        BP_HitIndicator();
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
            m_bonusCountDown = _rewardValue;
        break;

        case EMission::HoldGold:
            addEffect(EEffect::PassiveSpeed);
            m_bonusSpeed = _rewardValue;
        break;
    }
}

bool AShipPawn::canTank(int32 _val)
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
        lb_addScore(_val);
        return true;
    }
    else if (hasEffect(EEffect::Respawned) || hasEffect(EEffect::StartGame))
    {
        return true;
    }
    else
    {
        int32 newValue = this->R_ShieldLife - _val;
        newValue = FMath::Max(0, newValue);

        lb_addScore(this->R_ShieldLife - newValue);

        if (newValue == 0 && this->R_ShieldLife > 0)
        {
            removeEffect(EEffect::Shield);
            if (this->ShieldComponent != nullptr)
            {
                this->ShieldComponent->SetVisibility(false);
            }
        }

        this->R_ShieldLife = newValue;
        return this->R_ShieldLife > 0;
    }
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

        if (!hasEffect(EEffect::Shield))
        {
            this->ShieldComponent->SetVisibility(false);
        }
    }
}

void AShipPawn::RPCClientAddEffect_Implementation(EEffect _effect)
{
    OnAddEffectDelegate.Broadcast(_effect);
    BP_FxAddEffect(_effect);

    if (_effect == EEffect::BackToGame)
    {
        // TO DO Text system
        OnSendInfoPlayerDelegate.Broadcast("Didn't you seriously want to quit?");
    }
    else if (_effect == EEffect::MetaFormSupport)
    {
        if (this->PlayerDataAsset != nullptr)
        {
            FTransform const& transform = this->GetActorTransform();
            if (APostProcessInvisible* actor = Cast<APostProcessInvisible>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this->GetWorld(), this->PlayerDataAsset->MetaSupportPostProcessClass, transform)))
            {
                actor->Effect = _effect;
                OnRemoveEffectDelegate.AddDynamic(actor, &APostProcessInvisible::OnRemoveEffect);
                UGameplayStatics::FinishSpawningActor(actor, transform);
                actor->AttachToActor(this, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));
            }
        }
    }
    else if (_effect == EEffect::MetaFormAttack)
    {
        if (UModuleComponent* moduleComponent = this->ModuleComponent)
        {
            if (UInstancedStaticMeshComponent* weaponMeshComponent = this->ModuleComponent->WeaponMeshComponent)
            {
                weaponMeshComponent->SetScalarParameterValueOnMaterials("Edge sharp max", 0.0f);
                weaponMeshComponent->SetScalarParameterValueOnMaterials("Glow", 1200.0f);
            }
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
        m_countDownRespawn = 10;
        CountDownRespawn();
    }
}

void AShipPawn::CountDownRespawn()
{
    OnSendInfoPlayerDelegate.Broadcast("Rebuilding... " + FString::FromInt(m_countDownRespawn));
    --m_countDownRespawn;

    if (m_countDownRespawn > 0)
    {
        FTimerHandle handle;
        this->GetWorldTimerManager().SetTimer(handle, this, &AShipPawn::CountDownRespawn, 1.0f, false);
    }
}

void AShipPawn::RPCClientRemoveEffect_Implementation(EEffect _effect)
{
    OnRemoveEffectDelegate.Broadcast(_effect);
    BP_FxRemoveEffect(_effect);

    if (_effect == EEffect::MetaFormAttack)
    {
        if (UModuleComponent* moduleComponent = this->ModuleComponent)
        {
            if (UInstancedStaticMeshComponent* weaponMeshComponent = this->ModuleComponent->WeaponMeshComponent)
            {
                weaponMeshComponent->SetScalarParameterValueOnMaterials("Edge sharp max", 550.0f);
                weaponMeshComponent->SetScalarParameterValueOnMaterials("Glow", 800.0f);
            }
        }
    }
}

void AShipPawn::behaviourAddEffect(EEffect _type)
{
    if (_type == EEffect::Shield)
    {
        if (this->PlayerDataAsset != nullptr && !hasEffect(EEffect::Killed))
        {
            this->R_ShieldLife = this->PlayerDataAsset->ShieldLife;
            if (this->ShieldComponent != nullptr && !hasEffect(EEffect::MetaFormSupport))
            {
                this->ShieldComponent->SetVisibility(true);
            }
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
            RPCNetMulticastFxKilled();
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
    }
    else if (_type == EEffect::StartGame)
    {
        if (this->PlayerDataAsset != nullptr)
        {
            this->R_OverDrive = this->PlayerDataAsset->CoefOverDriveValue * 10;
        }
    }
    else if (_type == EEffect::Gold)
    {
        RPCNetMultiCastFxGold(true);
    }
}

void AShipPawn::RPCNetMultiCastFxGold_Implementation(bool _activate)
{
    BP_GoldFx(_activate);
}

void AShipPawn::RPCNetMulticastFxKilled_Implementation()
{
    BP_FxKilled();
}

void AShipPawn::BackToGame()
{
    removeEffect(EEffect::BackToGame);
}

void AShipPawn::addEffectSuccess(EEffect _type)
{
    behaviourAddEffect(_type);
    RPCClientAddEffect(_type);
}

void AShipPawn::removeEffectSuccess(EEffect _type)
{
    behaviourRemoveEffect(_type);
    RPCClientRemoveEffect(_type);
}

void AShipPawn::behaviourRemoveEffect(EEffect _type)
{
    if (_type == EEffect::Shield)
    {
        this->R_ShieldLife = 0;
        if (this->ShieldComponent != nullptr)
        {
            this->ShieldComponent->SetVisibility(false);
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
    }
    else if (_type == EEffect::TargetLock)
    {
        if (ACommonPawn* commonPawn = Cast<ACommonPawn>(this->FireComponent->m_target))
        {
            if (APlayerState* playerState = this->GetPlayerState<APlayerState>())
            {
                commonPawn->removePlayerFocusOnMe(playerState->PlayerId);
            }
        }
        this->FireComponent->m_target = nullptr;
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
}

void AShipPawn::RPCClientFeedbackScore_Implementation(EScoreType _type, int16 _value)
{
    OnFeedbackScoreDelegate.Broadcast(_type, _value);
}

void AShipPawn::RPCNetMulticastFxExploseHeal_Implementation()
{
    BP_ExploseHealFx();
}

void AShipPawn::heal(uint8 _value)
{
    this->RepairComponent->heal(_value);
    RPCNetMulticastFxExploseHeal();
}

ESkillReturn AShipPawn::onRepairProtection()
{
    if (this->RepairComponent != nullptr)
    {
        return this->RepairComponent->onRepairProtection();
    }
    return ESkillReturn::InternError;
}

ESkillReturn AShipPawn::onRepairSupport()
{
    if (this->RepairComponent != nullptr)
    {
        return this->RepairComponent->onRepairSupport();
    }
    return ESkillReturn::InternError;
}

void AShipPawn::addMatiere(int32 _val, EMatiereOrigin _type)
{
    this->RU_Matiere += _val;

    // in fact, _val > 0 is an useless test, because we have a type for lost matiere
    // therefor, we convert _val into uint so it's better to check if is not an negative number
    // because if in futur we have a case where we can have negative value for this type...
    if (_val > 0 && (_type == EMatiereOrigin::Farm || _type == EMatiereOrigin::Kill))
    {
        if (UMetricComponent* component = Cast<UMetricComponent>(this->GetComponentByClass(UMetricComponent::StaticClass())))
        {
            component->createMatiereWinData((uint16)_val);
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
            int healPackMatiere = uniqueSkillDataAsset->Value;
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

void AShipPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AShipPawn, RU_Matiere);
    DOREPLIFETIME(AShipPawn, R_ShieldLife);
    DOREPLIFETIME(AShipPawn, R_Effect);
}

