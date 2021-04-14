// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/Common/CommonPawn.h"
#include "Util/EnumUtil.h"
#include <functional>
#include "ShipPawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpdateMatiere, int, _value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndUpdateMatiere, int32, _value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitProtection);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitSupport);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRepairProtection);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRepairSupport);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShowScore, bool, _show);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLocalTeamUpdate, FString const&, _team);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAddEffect, EEffect, _type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemoveEffect, EEffect, _type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFeedbackScore, EScoreType, _type, int32, _value);

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
    void addEffect(EEffect _type);
    void removeEffect(EEffect _type);
    void behaviourAddEffect(EEffect _type);
    void behaviourRemoveEffect(EEffect _type);

    void launchMissile();
    void emp();
    void emp(uint32 _duration, FName const& _team, int32 _playerId);
    void giveMatiereToAlly(uint8 _id);

    UFUNCTION(UnReliable, Client)
    void RPCClientFeedbackScore(EScoreType _type, int16 _value);

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

    void useSkill(float _slot);
    bool canTank(int32 _val);

    UFUNCTION()
    void CleanEmp();

    UFUNCTION()
    void LinkPawn();

    UFUNCTION()
    void BackToGame();

public:
    UPROPERTY()
    FName Team {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnRepairProtection OnRepairProtectionDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnRepairSupport OnRepairSupportDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnFeedbackScore OnFeedbackScoreDelegate {};

protected:
    UPROPERTY(ReplicatedUsing = "OnRep_Matiere")
    int32 RU_Matiere { 0 };

    UPROPERTY(Replicated)
    int32 R_ShieldLife { 0 };

    FName m_lastTeamEmp {};
    int32 m_lastPlayerIdEmp {};

    FTransform m_startTransform {};

private:
    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnUpdateMatiere OnUpdateMatiereDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnHitProtection OnHitProtectionDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnHitSupport OnHitSupportDelegate {};

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
