// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Util/EnumUtil.h"
#include "Enum/SpacelEnum.h"
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

UCLASS()
class HUB_SPACEL_API AShipPawn : public APawn
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
	// Sets default values for this pawn's properties
	AShipPawn();

    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float _deltaTime) override;

    /* set collision profile name */
    void setCollisionProfile(FString _team);

    void hit(FString const& _team, class UPrimitiveComponent* _comp, int32 _index);

    void setLocationExhaustFx(TArray<FVector> const& _loc);

    float getPercentProtection() const;
    float getPercentSupport() const;

    /* client side */
    void lockTarget(int32 _playerId, bool _lock);

    /* server side */
    void addEffect(EEffect _type);
    void removeEffect(EEffect _type);
    bool hasEffect(EEffect _type);
    void behaviourAddEffect(EEffect _type);
    void behaviourRemoveEffect(EEffect _type);

    void launchMissile();
    void emp();
    void emp(uint32 _duration);

private:
    void lookAt(FVector const& _loc, FVector const& _dir, FVector const& _hitLoc);

    /* move ship server, only from server */
    void serverMove(float _deltaTime);

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

    template<class T>
    inline void activateComponent(T* _comp)
    {
        if (!ensure(_comp != nullptr)) return;
        _comp->Activate();
    }

    UFUNCTION()
    void OnRep_Matiere();

    UFUNCTION()
    void OnRep_PercentSpeed();

    UFUNCTION()
    void OnLockPrepare();

    UFUNCTION()
    void OnStartGame();

    UFUNCTION()
    void OnPlayerEnterFog(int32 _playerId, bool _enter);

    UFUNCTION(Reliable, NetMulticast)
    void RPCNetMulticastEnterFog(int32 _playerId, bool _enter);

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
    UPROPERTY(Category = "Ship", VisibleAnywhere, BlueprintReadOnly)
    class UStaticMeshComponent* DriverMeshComponent { nullptr };

    UPROPERTY(Category = "Ship", VisibleAnywhere, BlueprintReadOnly)
    class UStaticMeshComponent* ShieldComponent { nullptr };

    UPROPERTY(Category = "Ship", VisibleAnywhere, BlueprintReadOnly)
    class UPoseableMeshComponent* BaseShipMeshComponent { nullptr };

    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadOnly)
    class USpringArmComponent* SpringArmComponent { nullptr };

    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadOnly)
    class UCameraComponent* CameraComponent { nullptr };

    UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
    class UPlayerDataAsset* PlayerDataAsset { nullptr };

    /* module setup by server replicated to client, static mesh instance don't support replication */
    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class UModuleComponent* ModuleComponent { nullptr };

    /* only on server side */
    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class UFireComponent* FireComponent { nullptr };

    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class USkillComponent* SkillComponent { nullptr };

    /* only on server side */
    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class URepairComponent* RepairComponent { nullptr };

    UPROPERTY(Category = "ChildActor", EditAnywhere, BlueprintReadWrite)
    class UChildActorComponent* TargetComponent { nullptr };

    UPROPERTY(Category = "ChildActor", EditAnywhere, BlueprintReadWrite)
    TSubclassOf<class ATargetActor> TargetClass { nullptr };

    UPROPERTY(Category = "Component", EditAnywhere, BlueprintReadWrite)
    class UWidgetInteractionComponent* WidgetTargetComponent { nullptr };

    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class UPostProcessComponent* SpeedLinesComponent { nullptr };

    UPROPERTY(Category = "FX", EditAnywhere, BlueprintReadWrite)
    TSubclassOf<class UCameraShake> CameraShakeClass { nullptr };

    UPROPERTY(Category = "FX", EditAnywhere)
    class UMaterialInstance* MaterialSpeedLines { nullptr };

    UPROPERTY()
    FName Team {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnRepairProtection OnRepairProtectionDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnRepairSupport OnRepairSupportDelegate {};

protected:
    /* current percent speed value 0.0f - 1.0f */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = "OnRep_PercentSpeed")
    float RU_PercentSpeed { 0.0f };

    /* use by server */
    float PercentHorizontalStraf { 0.0f };
    float PercentVerticalStraf { 0.0f };
    float PercentFlightAttitude { 0.0f };
    FVector TargetLocation { FVector::ZeroVector };

    UPROPERTY(ReplicatedUsing = "OnRep_Matiere")
    int32 RU_Matiere { 0 };

    UPROPERTY()
    class UNiagaraComponent* ExhaustFxComponent { nullptr };

    UPROPERTY(Replicated)
    int32 R_ShieldLife { 0 };

    UPROPERTY(Replicated)
    EEffect R_Effect { EEffect::None };

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
