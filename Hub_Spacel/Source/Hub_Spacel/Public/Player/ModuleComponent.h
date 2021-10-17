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

    ESkillReturn onSwapEmergency(uint32 _nbMatiereUseForOne);

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

    /* build ship in lobby */
    void buildLobbyShip(ESkill _skillId, ESkillType _type);

    /* change meta form */
    void activeMetaForm(EEffect _type);
    void removeMetaForm(EEffect _type);

    /* activate bonus voxel on right component */
    void activateBonus(ESkill _skillId);

    /* get meta form matching with a skill */
    EFormType getFormType(ESkill _skillId) const;

    /* get meta form matching with an effect */
    EFormType getFormType(EEffect _type) const;

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

    template<class T>
    T* getPawn()
    {
        return Cast<T>(this->GetOwner());
    }

public:
    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnUpdateCountProtection OnUpdateCountProtectionDelegate {};

    UPROPERTY(BlueprintAssignable, Category = "EventDispatchers")
    FOnUpdateCountSupport OnUpdateCountSupportDelegate {};

private:
    /* form currently activated */
    TArray<EFormType> m_activatedForms;
};
