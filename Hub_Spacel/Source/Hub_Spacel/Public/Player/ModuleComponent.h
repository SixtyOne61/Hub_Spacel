// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Enum/SpacelEnum.h"
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

    ESkillReturn onSwapEmergency(uint32 _value, uint8 _tresholdPercent);

protected:
    template<class T>
    void createComponent(T*& _component, FName&& _name)
    {
        _component = CreateDefaultSubobject<T>(_name);
        if (!ensure(_component != nullptr)) return;

        _component->SetRenderCustomDepth(true);
        _component->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
    }

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

    /* server side */
    UFUNCTION()
    void OnChangeState(EGameState _state);

    UFUNCTION(BlueprintCallable, Category = "Components|Form")
    void UseForm(EFormType _type, bool _refresh);

    /* set collision profile name */
    void setCollisionProfile(FString _team);

    void buildLobbyShip(ESkill _skillId, ESkillType _type);

    void activeMetaForm(EEffect _type);
    void removeMetaForm(EEffect _type);

private:
    /* call ship pawn owner for set location of exhaust */
    UFUNCTION()
    void OnUpdateCountSupport(TArray<FVector_NetQuantize> const& _locations, int32 _max);

    UFUNCTION()
    void OnUpdateCountProtection(TArray<FVector_NetQuantize> const& _locations, int32 _max);

    UFUNCTION()
    void OnUpdateCountEmergency(TArray<FVector_NetQuantize> const& _locations, int32 _max);

    /* call when red zone is hit */
    void kill();
    /* call for when player need to be restarted */
    void restarted();

public:
    UPROPERTY(Category = "Component|Mesh", VisibleAnywhere, BlueprintReadWrite)
    class USpacelInstancedMeshComponent* EmergencyComponent { nullptr };

    UPROPERTY(Category = "Component|Mesh", VisibleAnywhere, BlueprintReadWrite)
    class USpacelInstancedMeshComponent* WeaponComponent{ nullptr };

    UPROPERTY(Category = "Component|Mesh", VisibleAnywhere, BlueprintReadWrite)
    class USpacelInstancedMeshComponent* ProtectionComponent{ nullptr };

    UPROPERTY(Category = "Component|Mesh", VisibleAnywhere, BlueprintReadWrite)
    class USpacelInstancedMeshComponent* SupportComponent{ nullptr };


    UPROPERTY(Category = "Component", VisibleAnywhere, BlueprintReadWrite)
    class UStaticMeshComponent* MissileMeshComponent { nullptr };

public:
    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnUpdateCountProtection OnUpdateCountProtectionDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnUpdateCountSupport OnUpdateCountSupportDelegate {};

private:

    UPROPERTY(Replicated)
    TArray<FVector_NetQuantize> R_MissileLocations {};

    /* form currently activated */
    TArray<EFormType> m_activatedForms;
};
