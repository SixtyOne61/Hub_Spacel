// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Enum/SpacelEnum.h"
#include "CommonPawn.generated.h"

UCLASS()
class HUB_SPACEL_API ACommonPawn : public APawn
{
	GENERATED_BODY()

	friend class ACommonPlayerController;

public:
	// Sets default values for this pawn's properties
	ACommonPawn();

	void lookAt(FVector const& _loc, FVector const& _dir, FVector const& _hitLoc);

    // effect part
    bool hasEffect(EEffect _type);

    /* set collision profile name */
    void setCollisionProfile(FString _team);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

    UFUNCTION()
    void OnRep_PercentSpeed();

    void moveShip(float _deltaTime);

    template<class T>
    inline void activateComponent(T* _comp)
    {
        if (!ensure(_comp != nullptr)) return;
        _comp->Activate();
    }

public:
    UPROPERTY(Category = "Ship", VisibleAnywhere, BlueprintReadOnly)
    class UStaticMeshComponent* DriverMeshComponent{ nullptr };

    UPROPERTY(Category = "Ship", VisibleAnywhere, BlueprintReadOnly)
    class UStaticMeshComponent* ShieldComponent{ nullptr };

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

    /* only on server side */
    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class UFireComponent* FireComponent{ nullptr };

    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class USkillComponent* SkillComponent{ nullptr };

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
    TSubclassOf<class UCameraShake> CameraShakeClass{ nullptr };

    UPROPERTY(Category = "FX", EditAnywhere)
    class UMaterialInstance* MaterialSpeedLines{ nullptr };

protected:
    UPROPERTY()
    class UNiagaraComponent* ExhaustFxComponent{ nullptr };

	FVector TargetLocation{ FVector::ZeroVector };

    UPROPERTY(Replicated)
    int32 R_Effect { 0 };

    /* current percent speed value 0.0f - 1.0f */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = "OnRep_PercentSpeed")
    float RU_PercentSpeed { 0.0f };

	/* use by server or offline */
	float PercentHorizontalStraf{ 0.0f };
	float PercentVerticalStraf{ 0.0f };
	float PercentFlightAttitude{ 0.0f };
};
