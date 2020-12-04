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
    /* move ship server */
    UFUNCTION(Reliable, Server)
    void RPCServerMove(float const& _deltaTime);

    /* move ship client */
    UFUNCTION(Unreliable, NetMulticast)
    void RPCClientMove(FVector const& _angularVelocity, FVector const& _linearVelocity);

    /* only efficient on server */
    void fire(float const& _deltaTime);

    virtual void OnRep_PlayerState() override;

    UFUNCTION()
    void StartGame();

    void buildShip(TArray<FVector> const& _redZoneLocations, TArray<FVector> const& _attackLocations, TArray<FVector> const& _protectionLocations, TArray<FVector> const& _supportLocations);

    UFUNCTION()
    void OnComponentBeginOverlap(UPrimitiveComponent* _overlappedComp, AActor* OtherActor, UPrimitiveComponent* _otherComp, int32 _otherBodyIndex, bool _bFromSweep, const FHitResult& _sweepResult);

    /* replication not supported on UInstancedStaticMeshComponent, call instance location on each client */
    UFUNCTION(Unreliable, NetMulticast)
    void RPCClientAddVoxel(TArray<FVector> const& _redZoneLocations, TArray<FVector> const& _attackLocations, TArray<FVector> const& _protectionLocations, TArray<FVector> const& _supportLocations);

    /* only use for debug in editor */
    UFUNCTION(BlueprintCallable)
    void BuildDefaultShip();

public:
    UPROPERTY(Category = "Ship", VisibleAnywhere, BlueprintReadOnly)
    class UStaticMeshComponent* DriverMeshComponent { nullptr };

    UPROPERTY(Category = "Ship", VisibleAnywhere, BlueprintReadOnly)
    class UPoseableMeshComponent* BaseShipMeshComponent { nullptr };

    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadOnly)
    class USpringArmComponent* SpringArmComponent { nullptr };

    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadOnly)
    class UCameraComponent* CameraComponent { nullptr };

    UPROPERTY(Category = "DataAsset", VisibleAnywhere, BlueprintReadWrite)
    class UPlayerDataAsset* PlayerDataAsset { nullptr };

    UPROPERTY(Category = "DataAsset", VisibleAnywhere, BlueprintReadWrite)
    class UStaticMeshDataAsset* RedZoneDataAsset { nullptr };

    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class UInstancedStaticMeshComponent* RedZoneMeshComponent { nullptr };

    UPROPERTY(Category = "DataAsset", VisibleAnywhere, BlueprintReadWrite)
    class USetupAttributeDataAsset* ProtectionDataAsset { nullptr };

    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class UInstancedStaticMeshComponent* ProtectionMeshComponent{ nullptr };

    UPROPERTY(Category = "DataAsset", VisibleAnywhere, BlueprintReadWrite)
    class USetupAttributeDataAsset* WeaponDataAsset { nullptr };

    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class UInstancedStaticMeshComponent* WeaponMeshComponent { nullptr };

    UPROPERTY(Category = "DataAsset", VisibleAnywhere, BlueprintReadWrite)
    class USetupAttributeDataAsset* SupportDataAsset { nullptr };

    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class UInstancedStaticMeshComponent* SupportMeshComponent { nullptr };

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
};
