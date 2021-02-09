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

    friend class AShipPawn;
    friend class UCustomCollisionComponent;
    friend class URepairComponent;

public:	
	// Sets default values for this component's properties
	UModuleComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

    /* server side */
    UFUNCTION()
    void OnStartGame();

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

private:
    void buildShip(class UInstancedStaticMeshComponent*& _mesh, class UStaticMeshDataAsset* _staticMesh, TArray<FVector> const& _locations);

    void setPercentVelocity(float _percent);

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

    UPROPERTY(Category = "Fx", EditAnywhere, BlueprintReadWrite)
    TArray<class USceneComponent*> ExhaustComponents{};

public:
    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnUpdateCountProtection OnUpdateCountProtectionDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnUpdateCountSupport OnUpdateCountSupportDelegate {};

    UPROPERTY(EditAnywhere)
    class UNiagaraSystem* ExhaustFx { nullptr };

private:
    UPROPERTY(ReplicatedUsing = "OnRep_Attack")
    TArray<FVector> RU_AttackLocations{};

    UPROPERTY(ReplicatedUsing = "OnRep_Protection")
    TArray<FVector> RU_ProtectionLocations{};

    UPROPERTY(Replicated)
    TArray<FVector> R_RemovedProtectionLocations{};

    UPROPERTY(ReplicatedUsing = "OnRep_Support")
    TArray<FVector> RU_SupportLocations{};

    UPROPERTY(Replicated)
    TArray<FVector> R_RemovedSupportLocations{};

    /* max protection and support cube */
    int32 m_maxProtection { -1 };
    int32 m_maxSupport { -1 };
};
