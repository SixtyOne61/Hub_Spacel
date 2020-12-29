// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ModuleComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HUB_SPACEL_API UModuleComponent : public USceneComponent
{
	GENERATED_BODY()

    friend class AShipPawn;

public:	
	// Sets default values for this component's properties
	UModuleComponent();

    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

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

private:
    void buildShip(class UInstancedStaticMeshComponent*& _mesh, class UStaticMeshDataAsset* _staticMesh, TArray<FVector> const& _locations);

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

private:
    UPROPERTY(ReplicatedUsing = "OnRep_Attack")
    TArray<FVector> RU_AttackLocations{};

    UPROPERTY(ReplicatedUsing = "OnRep_Protection")
    TArray<FVector> RU_ProtectionLocations{};

    UPROPERTY(ReplicatedUsing = "OnRep_Support")
    TArray<FVector> RU_SupportLocations{};
};
