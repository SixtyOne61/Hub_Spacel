// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ModuleComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpdateCountProtection, int32, _value, int32, _max);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpdateCountSupport, int32, _value, int32, _max);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HUB_SPACEL_API UModuleComponent : public USceneComponent
{
	GENERATED_BODY()

    friend class ACommonPawn;
    friend class AShipPawn;
    friend class ATutoPawn;
    friend class UCustomCollisionComponent;
    friend class URepairComponent;

public:	
	// Sets default values for this component's properties
	UModuleComponent();

    float getPercentProtection() const;
    float getPercentSupport() const;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

    /* server side */
    UFUNCTION()
    void OnStartGame(EGameState _state);

    UFUNCTION()
    void OnRep_Attack();

    UFUNCTION()
    void OnRep_Protection();

    UFUNCTION()
    void OnRep_Support();

    UFUNCTION(Reliable, Client)
    void SetMax(int32 _maxProtection, int32 _maxSupport);

    /* set collision profile name */
    void setCollisionProfile(FString _team);

    UFUNCTION()
    void BuildShipLobby();

private:
    void buildShip(class UInstancedStaticMeshComponent*& _mesh, class UStaticMeshDataAsset* _staticMesh, TArray<FVector_NetQuantize> const& _locations);

    /* call ship pawn owner for set location of exhaust */
    void setLocationExhaustFx();

    /* call when red zone is hit */
    void kill();
    /* call for when player need to be restarted */
    void restarted();

public:
    UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
    class USetupAttributeDataAsset* ProtectionDataAsset{ nullptr };

    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class UInstancedStaticMeshComponent* ProtectionMeshComponent{ nullptr };

    UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
    class USetupAttributeDataAsset* WeaponDataAsset{ nullptr };

    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class UInstancedStaticMeshComponent* WeaponMeshComponent{ nullptr };

    UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
    class USetupAttributeDataAsset* SupportDataAsset{ nullptr };

    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class UInstancedStaticMeshComponent* SupportMeshComponent{ nullptr };

    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class UStaticMeshComponent* MissileMeshComponent { nullptr };

public:
    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnUpdateCountProtection OnUpdateCountProtectionDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnUpdateCountSupport OnUpdateCountSupportDelegate {};

private:
    UPROPERTY(ReplicatedUsing = "OnRep_Attack")
    TArray<FVector_NetQuantize> RU_AttackLocations{};

    UPROPERTY()
    TArray<FVector_NetQuantize> RemovedAttackLocations{};

    UPROPERTY(ReplicatedUsing = "OnRep_Protection")
    TArray<FVector_NetQuantize> RU_ProtectionLocations{};

    UPROPERTY()
    TArray<FVector_NetQuantize> RemovedProtectionLocations{};

    UPROPERTY(ReplicatedUsing = "OnRep_Support")
    TArray<FVector_NetQuantize> RU_SupportLocations{};

    UPROPERTY()
    TArray<FVector_NetQuantize> RemovedSupportLocations{};

    UPROPERTY(Replicated)
    TArray<FVector_NetQuantize> R_MissileLocations {};

    /* max protection and support cube */
    int32 m_maxProtection { -1 };
    int32 m_maxSupport { -1 };
};
