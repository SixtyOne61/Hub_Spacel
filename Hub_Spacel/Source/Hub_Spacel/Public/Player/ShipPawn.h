// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Util/Optional.h"
#include "ShipPawn.generated.h"

UCLASS()
class HUB_SPACEL_API AShipPawn : public APawn
{
    struct FTempArray
    {
        TArray<FVector> RedZone {};
        TArray<FVector> Attack{};
        TArray<FVector> Protection{};
        TArray<FVector> Support{};
    };

	GENERATED_BODY()

    friend class APlayerShipController;
    friend class USpacelWidget;

public:
	// Sets default values for this pawn's properties
	AShipPawn();

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
    /* move ship server */
    UFUNCTION(Reliable, Server)
    void RPCServerMove(float const& _deltaTime);

    /* move ship client */
    UFUNCTION(Unreliable, Client)
    void RPCClientMove(FVector const& _angularVelocity, FVector const& _linearVelocity);

    /* Collision part */
    void handSweep();

    bool itemHits(TArray<FHitResult> const& _hits);

    /* only efficient on server */
    void fire(float const& _deltaTime);

    virtual void OnRep_PlayerState() override;

    UFUNCTION()
    void StartGame();

    void buildShip(TArray<FVector> const& _redZoneLocations, TArray<FVector> const& _attackLocations, TArray<FVector> const& _protectionLocations, TArray<FVector> const& _supportLocations);

    UFUNCTION()
    void OnComponentHit(UPrimitiveComponent* _hitComp, AActor* _otherActor, UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit);

    /* replication not supported on UInstancedStaticMeshComponent, call instance location on each client */
    UFUNCTION(Unreliable, Client)
    void RPCClientAddVoxel(TArray<FVector> const& _redZoneLocations, TArray<FVector> const& _attackLocations, TArray<FVector> const& _protectionLocations, TArray<FVector> const& _supportLocations);

    /* only use for debug in editor */
    UFUNCTION(BlueprintCallable)
    void BuildDefaultShip();

    /* target system */
    UFUNCTION()
    void OnTargetPlayer(class AActor* _target);

    UFUNCTION()
    void OnUnTargetPlayer();

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

    UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
    class USetupAttributeDataAsset* ProtectionDataAsset { nullptr };

    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class UInstancedStaticMeshComponent* ProtectionMeshComponent{ nullptr };

    UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
    class USetupAttributeDataAsset* WeaponDataAsset { nullptr };

    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class UInstancedStaticMeshComponent* WeaponMeshComponent { nullptr };

    UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
    class USetupAttributeDataAsset* SupportDataAsset { nullptr };

    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class UInstancedStaticMeshComponent* SupportMeshComponent { nullptr };

    UPROPERTY(Category = "ChildActor", EditAnywhere, BlueprintReadWrite)
    class UChildActorComponent* TargetComponent { nullptr };

    UPROPERTY(Category = "ChildActor", EditAnywhere, BlueprintReadWrite)
    TSubclassOf<class ATargetActor> TargetClass { nullptr };

protected:
    /* current percent speed value 0.0f - 1.0f */
    UPROPERTY(Replicated)
    float R_PercentSpeed = 0.0f;

    /* when flight attitude change -1.0f or 0.0f or 1.0f */
    UPROPERTY(ReplicatedUsing = "OnRep_PercentFlightAttitude")
    float RU_PercentFlightAttitude = 0.0f;

    /* when turn change -1.0f or 0.0f or 1.0f */
    UPROPERTY(ReplicatedUsing = "OnRep_PercentTurn")
    float RU_PercentTurn = 0.0f;

    /* when up change -1.0f or 0.0f or 1.0f */
    UPROPERTY(ReplicatedUsing = "OnRep_PercentUp")
    float RU_PercentUp = 0.0f;

    /* use only on server, say if we are in fire */
    Util::Optional<bool> m_isFire { };

    /* current time between next bullet */
    float m_fireCountDown { };

    /* fire point location */
    int32 m_fireIndex { };

    /* current targeted actor */
    class AActor* m_target { nullptr };
};
