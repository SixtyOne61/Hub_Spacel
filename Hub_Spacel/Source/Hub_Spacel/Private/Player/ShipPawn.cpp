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
#include "Player/SpacelPlayerState.h"
#include "Player/TargetActor.h"
#include "Player/FireComponent.h"
#include "Player/ModuleComponent.h"
#include "Player/CustomCollisionComponent.h"
#include "Player/GamePlayerController.h"
#include "Player/RepairComponent.h"
#include "Player/LocalPlayerActionComponent.h"
#include "Player/PlayerNameActor.h"
#include "GameState/SpacelGameState.h"
#include "Hub_SpacelGameInstance.h"
#include "Util/Tag.h"
#include "Util/SimplyMath.h"
#include "TimerManager.h"
#include "Gameplay/SkillComponent.h"
#include "NiagaraComponent.h"

void AShipPawn::OnLockPrepare()
{
    if (this->SkillComponent != nullptr)
    {
        this->SkillComponent->setupSkill();
    }
}

void AShipPawn::OnStartGame()
{
    // add custom collision component
    if (UCustomCollisionComponent* customCollisionComponent = NewObject<UCustomCollisionComponent>(this, "CustomCollision_00"))
    {
        customCollisionComponent->RegisterComponent();
    }

    m_startTransform = this->GetActorTransform();

    RPCClientStartGame(this->Team);
    RPCNetMulticastStartGame(this->Team);

    addEffect(EEffect::Respawned);
    FTimerDelegate timerCallback;
    timerCallback.BindLambda([&]() { removeEffect(EEffect::Respawned); });

    FTimerHandle handle;
    this->GetWorldTimerManager().SetTimer(handle, timerCallback, 3.0f, false);
}

void AShipPawn::RPCNetMulticastStartGame_Implementation(FName const& _team)
{
    if (this->ShieldComponent != nullptr)
    {
        if (this->TeamColorDataAsset != nullptr)
        {
            FColor color = this->TeamColorDataAsset->GetColor<FColor>(_team.ToString());
            this->ShieldComponent->SetVectorParameterValueOnMaterials("Color", FVector{ color.ReinterpretAsLinear() });
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
            spacelGameState->OnLockPrepareDelegate.AddDynamic(this, &AShipPawn::OnLockPrepare);
            spacelGameState->OnPlayerEnterFogDelegate.AddDynamic(this, &AShipPawn::OnPlayerEnterFog);
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
                spacelGameState->OnLockPrepareDelegate.AddDynamic(this, &AShipPawn::OnLockPrepare);
            }
            UHub_SpacelGameInstance* spacelGameInstance = Cast<UHub_SpacelGameInstance>(this->GetGameInstance());
            if (!ensure(spacelGameInstance != nullptr)) return;
            RPCServerSetPlayerName(spacelGameInstance->CustomPlayerName);
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

void AShipPawn::emp()
{
    if(this->FireComponent == nullptr) return;
    if (AShipPawn* target = Cast<AShipPawn>(this->FireComponent->m_target))
    {
        if(this->SkillComponent == nullptr) return;
        if(this->SkillComponent->SkillDataAsset == nullptr) return;
        uint32 duration = this->SkillComponent->SkillDataAsset->getSKill(ESkill::SpecialSupport).FlatDuration;
        if (ASpacelPlayerState* playerState = this->GetPlayerState<ASpacelPlayerState>())
        {
            target->emp(duration, this->Team, playerState->PlayerId);
        }
    }
}

void AShipPawn::emp(uint32 _duration, FName const& _team, int32 _playerId)
{
    m_lastTeamEmp = _team;
    m_lastPlayerIdEmp = _playerId;
    addEffect(EEffect::Emp);
    FTimerHandle handle;
    this->GetWorldTimerManager().SetTimer(handle, this, &AShipPawn::CleanEmp, _duration, false);
}

bool AShipPawn::giveMatiereToAlly(uint8 _id)
{
    if(this->RU_Matiere <= 0) return false;
    if(this->PlayerDataAsset == nullptr) return false;

    if (ASpacelPlayerState* localSpacelPlayerState = GetPlayerState<ASpacelPlayerState>())
    {
        FString const& localTeam = localSpacelPlayerState->R_Team;

        if (AGameStateBase* gameStateBase = GetWorld()->GetGameState())
        {
            TArray<APlayerState*> const& playerStates = gameStateBase->PlayerArray;
            uint8 i = 0;
            for (APlayerState const* playerState : playerStates)
            {
                if (ASpacelPlayerState const* spacelPlayerState = Cast<ASpacelPlayerState>(playerState))
                {
                    if(localSpacelPlayerState->PlayerId == playerState->PlayerId) continue;

                    if (spacelPlayerState->R_Team == localTeam)
                    {
                        if (i == _id)
                        {
                            if (AShipPawn* allyPawn = spacelPlayerState->GetPawn<AShipPawn>())
                            {
                                int value = FMath::Min(this->PlayerDataAsset->MaxGiveMatiere, this->RU_Matiere);
                                allyPawn->addMatiere(value);

                                addMatiere(value * -1);
                                return true;
                            }
                        }
                        ++i;
                    }
                }
            }
        }
    }

    return false;
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
        // move ship
        moveShip(_deltaTime);
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

        // disable collision
        setCollisionProfile("NoCollision");

        addEffect(EEffect::Killed);
        removeEffect(EEffect::Emp);
        removeEffect(EEffect::BackToGame);
        removeAllPlayerFocusOnMe();

        // replace actor to spawn
        this->SetActorTransform(m_startTransform);

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
        }
    }
}

void AShipPawn::OnRep_Matiere()
{
    if (this->OnEndUpdateMatiereDelegate.IsBound())
    {
        int32 delta = this->RU_Matiere - m_lastMatiere;
        FString str = FString::FromInt(delta);
        if (delta > 0)
        {
            FString signe { "+" };
            str = signe + str;
        }

        m_lastMatiere = this->RU_Matiere;
        this->OnEndUpdateMatiereDelegate.Broadcast(this->RU_Matiere, str);
    }
}

void AShipPawn::hit(FString const& _team, int32 _playerId, class UPrimitiveComponent* _comp, int32 _index)
{
    UCustomCollisionComponent* customCollisionComponent { Cast<UCustomCollisionComponent>(this->GetComponentByClass(UCustomCollisionComponent::StaticClass())) };
    if (customCollisionComponent != nullptr)
    {
        customCollisionComponent->hit(_team, _playerId, _comp, _index);
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
    else if (hasEffect(EEffect::Respawned))
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
}

void AShipPawn::RPCClientRemoveEffect_Implementation(EEffect _effect)
{
    OnRemoveEffectDelegate.Broadcast(_effect);
}

void AShipPawn::RPCClientStartMission_Implementation(FMission const& _mission)
{
    OnStartMissionDelegate.Broadcast(_mission);
}

void AShipPawn::RPCClientEndMission_Implementation(FMission const& _mission)
{
    OnEndMissionDelegate.Broadcast(_mission.Type);
}

void AShipPawn::RPCNetMulticastEndMission_Implementation(FMission const& _mission)
{
    OnEndMissionDelegate.Broadcast(_mission.Type);
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
}

void AShipPawn::RPCClientFeedbackScore_Implementation(EScoreType _type, int16 _value)
{
    OnFeedbackScoreDelegate.Broadcast(_type, _value);
}

bool AShipPawn::onRepairProtection()
{
    if (this->RepairComponent != nullptr)
    {
        return this->RepairComponent->onRepairProtection();
    }
    return false;
}

bool AShipPawn::onRepairSupport()
{
    if (this->RepairComponent != nullptr)
    {
        return this->RepairComponent->onRepairSupport();
    }
    return false;
}

void AShipPawn::addMatiere(int32 _val)
{
    this->RU_Matiere += _val;
    OnRep_Matiere();
}

void AShipPawn::farmAsteroide()
{
    if (this->PlayerDataAsset != nullptr)
    {
        m_nbAsteroideFarm++;
        if (this->PlayerDataAsset->NbAsteroideForMatiere <= m_nbAsteroideFarm)
        {
            addMatiere(1);
            m_nbAsteroideFarm = 0;
        }
    }
}

bool AShipPawn::spawnNinePack()
{
    if (this->PlayerDataAsset != nullptr)
    {
        if (this->RU_Matiere >= this->PlayerDataAsset->NbMatiereForNinePack)
        {
            addMatiere(this->PlayerDataAsset->NbMatiereForNinePack * -1);
            // spawn wall
            TArray<UActorComponent*> const& actors = this->GetComponentsByTag(USceneComponent::StaticClass(), "NinePack");
            if (actors.Num() > 0 && actors[0] != nullptr)
            {
                if (USceneComponent* comp = Cast<USceneComponent>(actors[0]))
                {
                    FTransform const& tr = comp->GetComponentTransform();
                    AActor* actor = Cast<AActor>(UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), this->PlayerDataAsset->NinePackClass, tr));
                    UGameplayStatics::FinishSpawningActor(actor, tr);
                    return true;
                }
            }
        }
    }
    return false;
}

void AShipPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AShipPawn, RU_Matiere);
    DOREPLIFETIME(AShipPawn, R_ShieldLife);
    DOREPLIFETIME(AShipPawn, R_Effect);
}

