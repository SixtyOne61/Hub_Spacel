// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/Common/CommonPawn.h"
#include "Util/EnumUtil.h"
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

public:
    // Called when the game starts or when spawned
    void BeginPlay() override;

	// Called every frame
	void Tick(float _deltaTime) override;

    void hit(FString const& _team, int32 _playerId, class UPrimitiveComponent* _comp, int32 _index);

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
    void emp();
    void emp(uint32 _duration, FName const& _team, int32 _playerId);
    bool giveMatiereToAlly(uint8 _id);

    bool onRepairProtection();
    bool onRepairSupport();

    UFUNCTION(UnReliable, Client)
    void RPCClientFeedbackScore(EScoreType _type, int16 _value);

    void addMatiere(int32 _val);
    void farmAsteroide();
    bool spawnNinePack();

private:
    void OnRep_PlayerState() override;

    /* only use for debug in editor */
    UFUNCTION(BlueprintCallable)
    void BuildDefaultShip();

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
    void OnLockPrepare();

    UFUNCTION()
    void OnStartGame();

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

    UFUNCTION(Reliable, Client)
    void RPCClientStartMission(FMission const& _mission);

    UFUNCTION(Reliable, Client)
    void RPCClientEndMission(FMission const& _mission);

    UFUNCTION(Reliable, NetMulticast)
    void RPCNetMulticastEndMission(FMission const& _mission);

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

protected:
    UPROPERTY(ReplicatedUsing = "OnRep_Matiere")
    int32 RU_Matiere { 0 };

    UPROPERTY(Replicated)
    int32 R_ShieldLife { 0 };

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
};
