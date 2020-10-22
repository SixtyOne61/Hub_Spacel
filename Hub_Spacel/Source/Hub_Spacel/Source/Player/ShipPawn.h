// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include <optional>
#include "ShipPawn.generated.h"

UCLASS()
class HUB_SPACEL_API AShipPawn : public APawn
{
	GENERATED_BODY()

    friend class APlayerShipController;

public:
	// Sets default values for this pawn's properties
	AShipPawn();

public:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float _deltaTime) override;

    UFUNCTION()
    void OnRep_PercentFlightAttitude();

    UFUNCTION()
    void OnRep_PercentTurn();

    UFUNCTION()
    void OnRep_PercentUp();

private:
    /* build ship with all module */
    UFUNCTION(BlueprintCallable)
    void BuildShip();

    /* build a module */
    void buildProceduralModule(class USpacelProceduralMeshComponent * _component, class UProceduralModuleDataAsset const* _module, FVector const& _location);

    /* move ship server */
    UFUNCTION(Reliable, Server)
    void RPCServerMove(float const& _deltaTime);

    /* move ship client */
    UFUNCTION(Unreliable, NetMulticast)
    void RPCClientMove(FVector const& _angularVelocity, FVector const& _linearVelocity);

    /* only efficient on server */
    void fire(float const& _deltaTime);

public:
    UPROPERTY(Category = "Ship", VisibleDefaultsOnly, BlueprintReadOnly)
    class UStaticMeshComponent* DriverMeshComponent = nullptr;

    UPROPERTY(Category = "Ship", VisibleDefaultsOnly, BlueprintReadOnly)
    class UPoseableMeshComponent* BaseShipMeshComponent = nullptr;

    UPROPERTY(Category = "Ship", VisibleDefaultsOnly, BlueprintReadOnly)
    class USpacelProceduralMeshComponent* ShipEngineComponent = nullptr;

    UPROPERTY(Category = "Ship", VisibleDefaultsOnly, BlueprintReadOnly)
    class USpacelProceduralMeshComponent* ShipShellComponent = nullptr;

    UPROPERTY(Category = "Ship", VisibleDefaultsOnly, BlueprintReadOnly)
    class UStaticMeshComponent* SubMachineComponent = nullptr;

    UPROPERTY(Category = "Component", VisibleDefaultsOnly, BlueprintReadOnly)
    class USpringArmComponent* SpringArmComponent = nullptr;

    UPROPERTY(Category = "Component", VisibleDefaultsOnly, BlueprintReadOnly)
    class UCameraComponent* CameraComponent = nullptr;

    UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
    class UPlayerDataAsset* PlayerDataAsset = nullptr;

    UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
    class UShipModuleDataAsset* ModuleDataAsset = nullptr;

protected:
    /* current percent speed value 0.0f - 1.0f */
    UPROPERTY(Replicated)
    float PercentSpeed = 0.0f;

    /* when flight attitude change -1.0f or 0.0f or 1.0f */
    UPROPERTY(ReplicatedUsing = "OnRep_PercentFlightAttitude")
    float PercentFlightAttitude = 0.0f;

    /* when turn change -1.0f or 0.0f or 1.0f */
    UPROPERTY(ReplicatedUsing = "OnRep_PercentTurn")
    float PercentTurn = 0.0f;

    /* when up change -1.0f or 0.0f or 1.0f */
    UPROPERTY(ReplicatedUsing = "OnRep_PercentUp")
    float PercentUp = 0.0f;

    /* use only on server, say if we are in fire */
    std::optional<bool> m_isFire { };

    /* current time between next bullet */
    float m_fireCountDown { };
};
