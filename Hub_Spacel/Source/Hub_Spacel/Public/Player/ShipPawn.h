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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRepairProtection, bool, _on);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRepairSupport, bool, _on);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStateEscapeModeChange, EEscapeMode, _state);

UCLASS()
class HUB_SPACEL_API AShipPawn : public APawn
{
	GENERATED_BODY()

    friend class APlayerShipController;
    friend class USpacelWidget;
    friend class UFireComponent;
    friend class UCustomCollisionComponent;
    friend class URepairComponent;
    friend class ULocalPlayerActionComponent;

public:
	// Sets default values for this pawn's properties
	AShipPawn();

    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float _deltaTime) override;

    /* set collision profile name */
    void setCollisionProfile(FString _team);

    /* call from server */
    void setIsInFog(bool _isIn);

    static int32 getPlayerIdFromTarget(AActor* _target);

    void hit(class UPrimitiveComponent* _comp, int32 _index);

    UFUNCTION(BlueprintCallable)
    void Restarted();

private:
    /* move ship server */
    UFUNCTION(Reliable, Server)
    void RPCServerMove(float const& _deltaTime);

    virtual void OnRep_PlayerState() override;

    /* only use for debug in editor */
    UFUNCTION(BlueprintCallable)
    void BuildDefaultShip();

    /* target system */
    UFUNCTION()
    void OnTargetPlayer(class AActor* _target);

    UFUNCTION(Reliable, Server)
    void RPCServerTargetPlayer(int32 _playerId);

    UFUNCTION()
    void OnUnTargetPlayer(class AActor* _target);

    UFUNCTION(Reliable, Server)
    void RPCServerUnTargetPlayer(int32 _playerId);

    void rpcTargetCall(class AActor* _target, std::function<void(int32)> _rpc);

    /* set fire boolean on component fire */
    void setFire(bool _on);

    /* call for kill a player when red zone is hit */
    void kill();

    template<class T>
    inline void activateComponent(T* _comp)
    {
        if (!ensure(_comp != nullptr)) return;
        _comp->Activate();
    }

    UFUNCTION()
    void OnRep_IsInFog();

    UFUNCTION()
    void OnRep_Matiere();

    UFUNCTION()
    void OnRep_PercentSpeed();

    UFUNCTION()
    void OnStartGame();

    /* trigger for make a fast move, only call from server */
    void TriggerEscapeMode();

    UFUNCTION()
    void SetTriggerEscapeMode(int32 _state);

    /* callback method when state change */
    void onChangeStateAvailable();
    void onChangeStateEscape();
    void onChangeStateCountDown();

    UFUNCTION(Reliable, Client)
    void RPCClientChangeStateEscapeMode(EEscapeMode _newState);

public:
    UPROPERTY(Category = "Ship", VisibleAnywhere, BlueprintReadOnly)
    class UStaticMeshComponent* DriverMeshComponent { nullptr };

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

protected:
    /* current percent speed value 0.0f - 1.0f */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = "OnRep_PercentSpeed")
    float RU_PercentSpeed = 0.0f;

    /* when flight attitude change -1.0f or 0.0f or 1.0f */
    UPROPERTY(Replicated)
    float R_PercentFlightAttitude = 0.0f;

    /* when turn change -1.0f or 0.0f or 1.0f */
    UPROPERTY(Replicated)
    float R_PercentTurn = 0.0f;

    /* when up change -1.0f or 0.0f or 1.0f */
    UPROPERTY(Replicated)
    float R_PercentUp = 0.0f;

    UPROPERTY(ReplicatedUsing = "OnRep_IsInFog")
    bool RU_IsInFog { false };

    UPROPERTY(ReplicatedUsing = "OnRep_Matiere")
    int32 RU_Matiere { 0 };

    /* state of escape mode phase; only server side */
    EnumUtil::EnumCallback<EEscapeMode> m_escapeModeState {};

private:
    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnUpdateMatiere OnUpdateMatiereDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnHitProtection OnHitProtectionDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnHitSupport OnHitSupportDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnRepairProtection OnRepairProtectionDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnRepairSupport OnRepairSupportDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnEndUpdateMatiere OnEndUpdateMatiereDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnStateEscapeModeChange OnStateEspaceModeChangeDelegate {};
};
