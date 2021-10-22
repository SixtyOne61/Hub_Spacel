// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/Common/CommonPawn.h"
#include "Util/EnumUtil.h"
#include "Util/SpacelEvent.h"
#include "Enum/SpacelEnum.h"
#include <functional>
#include "ShipPawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpdateMatiere, int, _value, EMatiereOrigin, _type);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEndUpdateMatiere, int32, _value, FString const&, _deltaStr);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShowMission, bool, _show);
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
    friend class UInGameWidget;
    friend class UFireComponent;
    friend class UCustomCollisionComponent;
    friend class URepairComponent;
    friend class ULocalPlayerActionComponent;
    friend class USkillComponent;
    friend class UMetricComponent;
    friend class UModuleComponent;
    friend class AMissionManager;
    friend class AComet;
    friend class UTargetUserWidget;
    friend class UEmergencyInstancedMeshComponent;

public:
    // Called when the game starts or when spawned
    void BeginPlay() override;

	// Called every frame
	void Tick(float _deltaTime) override;

    void hit(FString const& _team, int32 _playerId, class UPrimitiveComponent* _comp, int32 _index, FVector const& _otherLocation);

    void setLocationExhaustFx(TArray<FVector_NetQuantize> const& _loc);

    float getPercentProtection() const;
    float getPercentSupport() const;

    /* server side */
    void addEffectSuccess(EEffect _type) override;
    void removeEffectSuccess(EEffect _type) override;
    void behaviourAddEffect(EEffect _type);
    void behaviourRemoveEffect(EEffect _type);

    void spawnKatyusha();
    ESkillReturn spawnEmp();
    void emp(uint32 _duration, FName const& _team, int32 _playerId);

    ESkillReturn onRepair();
    ESkillReturn onSwapEmergency();
    void onEmergencyCountDownEnd();

    UFUNCTION(UnReliable, Client)
    void RPCClientFeedbackScore(EScoreType _type, int16 _value);

    void addMatiere(int32 _val, EMatiereOrigin _type);
    void farmAsteroide();
    ESkillReturn spawnHealPack();

    void boostPassive(EMission _type, int32 _rewardValue);

    void buildLobbyShip(ESkill _skillId, ESkillType _type);

protected:
    UFUNCTION(BlueprintImplementableEvent)
    void BP_OnStartGame();

    UFUNCTION(BlueprintImplementableEvent)
    void BP_FxAddMatiere(int32 _val);

    UFUNCTION(BlueprintImplementableEvent)
    void BP_FxAddEffect(EEffect _effect);

    UFUNCTION(BlueprintImplementableEvent)
    void BP_FxGlobalAddEffect(EEffect _effect);

    UFUNCTION(BlueprintImplementableEvent)
    void BP_FxGlobalRemoveEffect(EEffect _effect);

    UFUNCTION(BlueprintImplementableEvent)
    void BP_FxRemoveEffect(EEffect _effect);

    UFUNCTION(BlueprintImplementableEvent)
    void BP_InitFireComponent();

    UFUNCTION(BlueprintImplementableEvent)
    void BP_ExploseHealFx();

    UFUNCTION(BlueprintImplementableEvent)
    void BP_GoldFx(bool _activate);

    UFUNCTION(BlueprintImplementableEvent)
    void BP_SpeedSound(int _percentSpeed, bool _start);

    UFUNCTION(BlueprintImplementableEvent, Category = "Visual Fx")
    void BP_PlayFlash() const;

    UFUNCTION()
    void OnEndMission(EMission _type, bool _succeed, FName _succeedForTeam);

    void emergencyRedCube();

private:
    void OnRep_PlayerState() override;

    /* only use for debug in editor */
    UFUNCTION(BlueprintCallable)
    void BuildDefaultShip();

    /* set fire boolean on component fire */
    void setFire(bool _on);

    /* call for kill a player when red zone is hit */
    void kill();

    /* call for force heal */
    void heal(uint8 _value);

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
    void RPCClientPlayCameraShake(EImpactType _type);

    UFUNCTION(Reliable, Client)
    void RPCClientStartGame(FName const& _team);

    UFUNCTION(Reliable, Client)
    void RPCClientAddEffect(EEffect _effect);

    UFUNCTION(Reliable, NetMulticast)
    void RPCNetMulticastAddEffect(EEffect _effect);

    UFUNCTION(Reliable, NetMulticast)
    void RPCNetMulticastRemoveEffect(EEffect _effect);

    UFUNCTION(Reliable, Client)
    void RPCClientRemoveEffect(EEffect _effect);

    UFUNCTION(UnReliable, NetMulticast)
    void RPCNetMulticastFxExploseHeal();

    UFUNCTION(UnReliable, NetMulticast)
    void RPCNetMultiCastFxGold(bool _activate);

    UFUNCTION(Reliable, Server)
    void RPCServerSendTarget(int32 _playerId);

    UFUNCTION(Reliable, Server)
    void RPCServerResetTarget();

    bool canTank(int32 _val);

    UFUNCTION()
    void CleanEmp();

    UFUNCTION()
    void LinkPawn();

    UFUNCTION()
    void BackToGame();

    void computeSoundData();

    /* if effect need to play flash, call bp function */
    void playFash(EEffect _effect) const;

public:
    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnFeedbackScore OnFeedbackScoreDelegate {};

    using ConstStr = FString const&;
    Util::Event<ConstStr, ConstStr> OnKill {};

    Util::Event<> OnLostGoldDelegate{ };

protected:
    UPROPERTY(ReplicatedUsing = "OnRep_Matiere")
    int16 RU_Matiere { 0 };

    UPROPERTY(Replicated)
    int8 R_ShieldLife { 0 };

    FName m_lastTeamEmp {};
    int32 m_lastPlayerIdEmp {};

    short m_lastDirection { 0 };

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FTransform StartTransform {};

    // use by server
    int m_nbAsteroideFarm {0};

    // use by local client for feedback
    int32 m_lastMatiere {0};

    UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
    class UMissionDataAsset* MissionDataAsset { nullptr };

    UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
    class UEditorHackDataAsset* HackDataAsset { nullptr };

    UPROPERTY(EditAnywhere)
    class UNiagaraSystem* HealPackFx { nullptr };

private:
    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnUpdateMatiere OnUpdateMatiereDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnEndUpdateMatiere OnEndUpdateMatiereDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnShowMission OnShowMissionDelegate {}; // deprecated to do remove

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnLocalTeamUpdate OnLocalTeamUpdateDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnAddEffect OnAddEffectDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnRemoveEffect OnRemoveEffectDelegate {};

    // only for local player
    FRotator m_defaultSprintArmRotator {};

    // false when endgame appear
    bool m_endGame { false };
};
