// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Util/Optional.h"
#include "ShipPawn.generated.h"

UCLASS()
class HUB_SPACEL_API AShipPawn : public APawn
{
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

    void buildRedZone();
    void buildAttack(uint8 _level);
    void buildProtection(uint8 _level);
    void buildSupport(uint8 _level);

    void addVoxelFromXml(class UInstancedStaticMeshComponent* _mesh, FString const& _filePath);

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
    class URedZoneDataAsset* RedZoneDataAsset { nullptr };

    UPROPERTY(Category = "RedZone", EditAnywhere, BlueprintReadWrite)
    class UInstancedStaticMeshComponent* RedZoneMeshComponent { nullptr };

    UPROPERTY(Category = "Protection", EditAnywhere, BlueprintReadWrite)
    class UProtectionDataAsset* ProtectionDataAsset { nullptr };

    UPROPERTY(Category = "Protection", EditAnywhere, BlueprintReadWrite)
    class UInstancedStaticMeshComponent* ProtectionMeshComponent{ nullptr };

    UPROPERTY(Category = "Weapon", EditAnywhere, BlueprintReadWrite)
    class UWeaponDataAsset* WeaponDataAsset { nullptr };

    UPROPERTY(EditAnywhere, Category = "Weapon")
    class UInstancedStaticMeshComponent* WeaponMeshComponent { nullptr };

    UPROPERTY(Category = "Support", EditAnywhere, BlueprintReadWrite)
    class USupportDataAsset* SupportDataAsset { nullptr };

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
    TQueue<FVector> m_fireLocations { };
};
