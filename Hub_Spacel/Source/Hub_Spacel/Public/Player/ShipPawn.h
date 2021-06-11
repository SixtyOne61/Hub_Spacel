// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/Common/CommonPawn.h"
#include "Util/EnumUtil.h"
#include "Util/SpacelEvent.h"
#include "DataAsset/MissionDataAsset.h"
#include <functional>
#include "ShipPawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpdateMatiere, int, _value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEndUpdateMatiere, int32, _value, FString const&, _deltaStr);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShowScore, bool, _show);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLocalTeamUpdate, FString const&, _team);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAddEffect, EEffect, _type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemoveEffect, EEffect, _type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFeedbackScore, EScoreType, _type, int32, _value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStartMission, FMission const&, _mission);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndMission, EMission, _type);

UCLASS()
class HUB_SPACEL_API AShipPawn : public ACommonPawn
{
	GENERATED_BODY()

    friend class AGamePlayerController;
    friend class USpacelWidget;
    friend class UFireComponent;
    friend class UCustomCollisionComponent;
    friend class URepairComponent;
    friend class ULocalPlayerActionComponent;
    friend class USkillComponent;
    friend class AMissionManager;
    friend class AComet;

public:
    // Called when the game starts or when spawned
    void BeginPlay() override;

	// Called every frame
	void Tick(float _deltaTime) override;

    void hit(FString const& _team, int32 _playerId, class UPrimitiveComponent* _comp, int32 _index, FVector const& _otherLocation);

    void setLocationExhaustFx(TArray<FVector> const& _loc);

    float getPercentProtection() const;
    float getPercentSupport() const;

    /* client side */
    void lockTarget(int32 _playerId, bool _lock);

    /* server side */
    void addEffectSuccess(EEffect _type) override;
    void removeEffectSuccess(EEffect _type) override;
    void behaviourAddEffect(EEffect _type);
    void behaviourRemoveEffect(EEffect _type);

    void launchMissile();
    void spawnKatyusha();
    void emp();
    void emp(uint32 _duration, FName const& _team, int32 _playerId);
    ESkillReturn giveMatiereToAlly(uint8 _id);

    ESkillReturn onRepairProtection();
    ESkillReturn onRepairSupport();

    UFUNCTION(UnReliable, Client)
    void RPCClientFeedbackScore(EScoreType _type, int16 _value);

    void addMatiere(int32 _val);
    void farmAsteroide();
    ESkillReturn spawnNinePack();

    UFUNCTION(Reliable, Client)
    void RPCClientStartMission(FMission const& _mission);

    UFUNCTION(Reliable, Client)
    void RPCClientEndMission(FMission const& _mission);

    UFUNCTION(Reliable, NetMulticast)
    void RPCNetMulticastEndMission(FMission const& _mission);

    void boostWall();

protected:
    UFUNCTION(BlueprintImplementableEvent)
    void BP_OnStartGame();

    UFUNCTION(BlueprintImplementableEvent)
    void BP_FxFireBullet();

    UFUNCTION(BlueprintImplementableEvent)
    void BP_FxFireMissile();

    UFUNCTION(BlueprintImplementableEvent)
    void BP_FxSpawnNinePacks();

    UFUNCTION(BlueprintImplementableEvent)
    void BP_FxAddMatiere(int32 _val);

    UFUNCTION(BlueprintImplementableEvent)
    void BP_FxKilled();

    UFUNCTION(BlueprintImplementableEvent)
    void BP_FxAddEffect(EEffect _effect);

    UFUNCTION(BlueprintImplementableEvent)
    void BP_FxRemoveEffect(EEffect _effect);

    UFUNCTION(BlueprintImplementableEvent)
    void BP_InitFireComponent();

private:
    void OnRep_PlayerState() override;

    /* only use for debug in editor */
    UFUNCTION(BlueprintCallable)
    void BuildDefaultShip();

    UFUNCTION()
    void BuildShip();

    /* target system */
    UFUNCTION(Reliable, Server)
    void RPCServerTargetPlayer(int32 _playerId, bool _lock);

    /* set fire boolean on component fire */
    void setFire(bool _on);

    /* call for kill a player when red zone is hit */
    void kill();

    /* call on server */
    UFUNCTION()
    void Restarted();

    UFUNCTION()
    void OnRep_Matiere();

    UFUNCTION()
    void OnChangeState(EGameState _state);

    UFUNCTION()
    void OnPlayerEnterFog(int32 _playerId, bool _enter);

    UFUNCTION(Reliable, Server)
    void RPCServerSetPlayerName(FString const& _playerName);

    UFUNCTION(Reliable, NetMulticast)
    void RPCNetMulticastEnterFog(int32 _playerId, bool _enter);

    UFUNCTION(Reliable, NetMulticast)
    void RPCNetMulticastEnterHidding(int32 _playerId, bool _enter);

    UFUNCTION(Reliable, NetMulticast)
    void RPCNetMulticastStartGame(FName const& _team);

    UFUNCTION(UnReliable, Client)
    void RPCClientPlayCameraShake();

    UFUNCTION(Reliable, Client)
    void RPCClientStartGame(FName const& _team);

    UFUNCTION(Reliable, Client)
    void RPCClientAddEffect(EEffect _effect);

    UFUNCTION(Reliable, Client)
    void RPCClientRemoveEffect(EEffect _effect);

    UFUNCTION(UnReliable, NetMulticast)
    void RPCNetMulticastFxFireBullet();

    UFUNCTION(UnReliable, NetMulticast)
    void RPCNetMulticastFxFireMissile();

    UFUNCTION(UnReliable, NetMulticast)
    void RPCNetMulticastFxNinePack();

    UFUNCTION(UnReliable, NetMulticast)
    void RPCClientFxAddMatiere(int32 _val);

    UFUNCTION(UnReliable, NetMulticast)
    void RPCNetMulticastFxKilled();

    bool canTank(int32 _val);

    UFUNCTION()
    void CleanEmp();

    UFUNCTION()
    void LinkPawn();

    UFUNCTION()
    void BackToGame();

public:
    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnFeedbackScore OnFeedbackScoreDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnStartMission OnStartMissionDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnEndMission OnEndMissionDelegate {};

    using ConstStr = FString const&;
    Util::Event<ConstStr, ConstStr> OnKill {};

protected:
    UPROPERTY(ReplicatedUsing = "OnRep_Matiere")
    int32 RU_Matiere { 0 };

    UPROPERTY(Replicated)
    int32 R_ShieldLife { 0 };

    UPROPERTY(Replicated)
    bool R_HasBoostWall { false };

    FName m_lastTeamEmp {};
    int32 m_lastPlayerIdEmp {};

    FTransform m_startTransform {};

    // use by server
    int m_nbAsteroideFarm {0};

    // use by local client for feedback
    int32 m_lastMatiere {0};

private:
    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnUpdateMatiere OnUpdateMatiereDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnEndUpdateMatiere OnEndUpdateMatiereDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnShowScore OnShowScoreDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnLocalTeamUpdate OnLocalTeamUpdateDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnAddEffect OnAddEffectDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnRemoveEffect OnRemoveEffectDelegate {};

    // only for local player
    FRotator m_defaultSprintArmRotator {};
};
