// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Enum/SpacelEnum.h"
#include "CommonPawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSendInfoPlayer, FString const&, _text);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHitFeedback, TArray<FHitResult> const&, _hits);

UCLASS()
class HUB_SPACEL_API ACommonPawn : public APawn
{
	GENERATED_BODY()

	friend class ACommonPlayerController;
    friend class ULocalPlayerActionComponent;
    friend class UFireComponent;
    friend class SkillCountDown;

public:
	// Sets default values for this pawn's properties
	ACommonPawn();

    void setActorRotation(FRotator const& _rotator);

	void lookAt(FVector const& _loc, FVector const& _dir, FVector const& _hitLoc);

    // effect part
    bool hasEffect(EEffect _type);
    void addEffect(EEffect _type);
    virtual void addEffectSuccess(EEffect _type) {};
    void removeEffect(EEffect _type);
    virtual void removeEffectSuccess(EEffect _type) {};

    /* set collision profile name */
    void setCollisionProfile(FString _team);

    UFUNCTION(UnReliable, Client)
    void RPCClientDamageIndicator(FVector_NetQuantize const& _location);

    UFUNCTION(BlueprintImplementableEvent)
    void BP_DamageIndicator(FVector const& _location);

    UFUNCTION(BlueprintImplementableEvent)
    void BP_HitIndicator();

    void halfTurn();

protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;

    UFUNCTION()
    void OnRep_PercentSpeed();

    UFUNCTION()
    void OnRep_RightTrail();

    UFUNCTION()
    void OnRep_LeftTrail();

    void moveShip(float _deltaTime);

    template<class T>
    inline void activateComponent(T* _comp)
    {
        if (!ensure(_comp != nullptr)) return;
        _comp->Activate();
    }

public:
    UPROPERTY()
    FName Team {};

    UPROPERTY(Category = "Ship", VisibleAnywhere, BlueprintReadOnly)
    class UStaticMeshComponent* DriverMeshComponent{ nullptr };

    UPROPERTY(Category = "Ship", VisibleAnywhere, BlueprintReadOnly)
    class UPoseableMeshComponent* BaseShipMeshComponent{ nullptr };

    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadOnly)
    class USpringArmComponent* SpringArmComponent{ nullptr };

    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadOnly)
    class UCameraComponent* CameraComponent{ nullptr };

    UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
    class UPlayerDataAsset* PlayerDataAsset{ nullptr };

    UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
    class UTeamColorDataAsset* TeamColorDataAsset{ nullptr };

    /* module setup by server replicated to client, static mesh instance don't support replication */
    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class UModuleComponent* ModuleComponent{ nullptr };

    UPROPERTY(Category = "Component|Mesh", VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class UEmergencyInstancedMeshComponent* EmergencyComponent{ nullptr };

    UPROPERTY(Category = "Component|Mesh", VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class UAnimatedSpacelMeshComponent* WeaponComponent{ nullptr };

    UPROPERTY(Category = "Component|Mesh", VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class UAnimatedSpacelMeshComponent* ProtectionComponent{ nullptr };

    UPROPERTY(Category = "Component|Mesh", VisibleAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    class UAnimatedSpacelMeshComponent* SupportComponent{ nullptr };

    /* only on server side */
    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class UFireComponent* FireComponent{ nullptr };

    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class USkillComponent* SkillComponent{ nullptr };

    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class UMetricComponent* MetricComponent { nullptr };

    /* only on server side */
    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class URepairComponent* RepairComponent{ nullptr };

    UPROPERTY(Category = "ChildActor", EditAnywhere, BlueprintReadWrite)
    class UChildActorComponent* TargetComponent{ nullptr };

    UPROPERTY(Category = "ChildActor", EditAnywhere, BlueprintReadWrite)
    TSubclassOf<class ATargetActor> TargetClass{ nullptr };

    UPROPERTY(Category = "ChildActor", EditAnywhere, BlueprintReadWrite)
    class UChildActorComponent* PlayerNameComponent{ nullptr };

    UPROPERTY(Category = "ChildActor", EditAnywhere, BlueprintReadWrite)
    TSubclassOf<class APlayerNameActor> PlayerNameClass{ nullptr };

    UPROPERTY(Category = "Component", EditAnywhere, BlueprintReadWrite)
    class UWidgetInteractionComponent* WidgetTargetComponent{ nullptr };

    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class UPostProcessComponent* SpeedLinesComponent{ nullptr };

    UPROPERTY(Category = "FX", EditAnywhere, BlueprintReadWrite)
    TSubclassOf<class UCameraShake> CameraShakeObstacleClass{ nullptr };

    UPROPERTY(Category = "FX", EditAnywhere, BlueprintReadWrite)
    TSubclassOf<class UCameraShake> CameraShakeHitClass{ nullptr };

    UPROPERTY(Category = "FX", EditAnywhere)
    class UMaterialInstance* MaterialSpeedLines{ nullptr };

    UPROPERTY(Category = "FX", EditAnywhere, BlueprintReadWrite)
    class UNiagaraComponent* RightTrailComponent { nullptr };

    UPROPERTY(Category = "FX", EditAnywhere, BlueprintReadWrite)
    class UNiagaraComponent* LeftTrailComponent { nullptr };

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnSendInfoPlayer OnSendInfoPlayerDelegate {};

    UFUNCTION()
    void UnFreezeRotation();

protected:
    UPROPERTY()
    TArray<class UNiagaraComponent*> ExhaustFxComponents { };

	FVector TargetLocation{ FVector::ZeroVector };

    UPROPERTY(Replicated)
    int32 R_Effect { 0 };

    /* current percent speed value 0.0f - 1.0f */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = "OnRep_PercentSpeed")
    float RU_PercentSpeed { 0.0f };

    UPROPERTY(Replicated)
    uint8 R_OverDrive { 0 };

    UPROPERTY(ReplicatedUsing = "OnRep_RightTrail")
    bool RU_RightTrail{ false };

    UPROPERTY(ReplicatedUsing = "OnRep_LeftTrail")
    bool RU_LeftTrail { false };

	/* use by server or offline */
	float PercentHorizontalStraf{ 0.0f };
	float PercentVerticalStraf{ 0.0f };
	float PercentFlightAttitude{ 0.0f };

    // if we have bonus, value != 0
    // must replicated reduc count down for progress bar skill
    UPROPERTY(Replicated)
    uint8 R_BonusCountDown{ 0 };
    uint8 m_bonusFireRate{ 0 };
    uint8 m_bonusSpeed{ 0 };

private:
    UPROPERTY()
    TArray<int32> PlayerFocusOnYou{};

    bool m_freezeRotation{ false };
};
