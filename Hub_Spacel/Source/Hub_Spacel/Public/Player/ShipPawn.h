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

    void buildRedZone(TOptional<FTempArray> & _tmpArray);
    void buildAttack(TOptional<FTempArray>& _tmpArray, uint8 _level);
    void buildProtection(TOptional<FTempArray>& _tmpArray, uint8 _level);
    void buildSupport(TOptional<FTempArray>& _tmpArray, uint8 _level);

    void addVoxel(class UInstancedStaticMeshComponent* & _mesh, FVector && _location) const;
    void addVoxel(class UInstancedStaticMeshComponent* & _mesh, TArray<FVector>&& _locations) const;

    /* replication not supported on UInstancedStaticMeshComponent, call instance location on each client */
    UFUNCTION(Unreliable, NetMulticast)
    void RPCClientAddVoxel(TArray<FVector> const& _redZoneLocations, TArray<FVector> const& _attackLocations, TArray<FVector> const& _protectionLocations, TArray<FVector> const& _supportLocations);

    UFUNCTION()
    void OnComponentHitProtection(class UPrimitiveComponent* _hitComp, AActor* _otherActor, class UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit);

    UFUNCTION()
    void OnComponentHitRedZone(class UPrimitiveComponent* _hitComp, AActor* _otherActor, class UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit);

    UFUNCTION()
    void OnComponentHitSupport(class UPrimitiveComponent* _hitComp, AActor* _otherActor, class UPrimitiveComponent* _otherComp, FVector _normalImpulse, const FHitResult& _hit);

    UFUNCTION(BlueprintCallable)
    void BuildDefaultShip();

public:
    UPROPERTY(Category = "Ship", VisibleDefaultsOnly, BlueprintReadOnly)
    class UStaticMeshComponent* DriverMeshComponent { nullptr };

    UPROPERTY(Category = "Ship", VisibleDefaultsOnly, BlueprintReadOnly)
    class UPoseableMeshComponent* BaseShipMeshComponent { nullptr };

    UPROPERTY(Category = "Component", VisibleDefaultsOnly, BlueprintReadOnly)
    class USpringArmComponent* SpringArmComponent { nullptr };

    UPROPERTY(Category = "Component", VisibleDefaultsOnly, BlueprintReadOnly)
    class UCameraComponent* CameraComponent { nullptr };

    UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
    class UPlayerDataAsset* PlayerDataAsset { nullptr };

    UPROPERTY(Category = "RedZone", EditAnywhere, BlueprintReadWrite)
    class UStaticMeshDataAsset* RedZoneDataAsset { nullptr };

    UPROPERTY(Category = "RedZone", EditAnywhere, BlueprintReadWrite)
    class UInstancedStaticMeshComponent* RedZoneMeshComponent { nullptr };

    UPROPERTY(Category = "Protection", EditAnywhere, BlueprintReadWrite)
    class USetupAttributeDataAsset* ProtectionDataAsset { nullptr };

    UPROPERTY(Category = "Protection", EditAnywhere, BlueprintReadWrite)
    class UInstancedStaticMeshComponent* ProtectionMeshComponent{ nullptr };

    UPROPERTY(Category = "Weapon", EditAnywhere, BlueprintReadWrite)
    class USetupAttributeDataAsset* WeaponDataAsset { nullptr };

    UPROPERTY(EditAnywhere, Category = "Weapon")
    class UInstancedStaticMeshComponent* WeaponMeshComponent { nullptr };

    UPROPERTY(Category = "Support", EditAnywhere, BlueprintReadWrite)
    class USetupAttributeDataAsset* SupportDataAsset { nullptr };

    UPROPERTY(EditAnywhere, Category = "Support")
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
