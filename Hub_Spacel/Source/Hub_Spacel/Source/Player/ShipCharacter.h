// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include <optional>
#include "ShipCharacter.generated.h"

UCLASS()
class HUB_SPACEL_API AShipCharacter : public ACharacter
{
	GENERATED_BODY()

    friend class APlayerShipController;

public:
	// Sets default values for this character's properties
	AShipCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float _deltaTime) override;

private:
    /* move ship server */
    UFUNCTION(Reliable, Server)
    void RPCServerMove(float const& _deltaTime);

    /* move ship client */
    UFUNCTION(Unreliable, NetMulticast)
    void RPCClientMove(FVector const& _velocity, FRotator const& _deltaRotation);

    UFUNCTION(BlueprintCallable)
    void BuildShip();

    /* init all ship module and camera */
    void initShip();

    /* build a module */
    void buildProceduralModule(class USpacelProceduralMeshComponent * _component, class UProceduralModuleDataAsset const* _module, FVector const& _location);

public:
    UPROPERTY(Category = "Component", VisibleDefaultsOnly, BlueprintReadOnly)
    class USpringArmComponent* SpringArmComponent;

    UPROPERTY(Category = "Component", VisibleDefaultsOnly, BlueprintReadOnly)
    class UCameraComponent* CameraComponent;

    UPROPERTY(Category = "Ship", VisibleDefaultsOnly, BlueprintReadOnly)
    class USpacelProceduralMeshComponent* ShipEngineComponent = nullptr;

    UPROPERTY(Category = "Ship", VisibleDefaultsOnly, BlueprintReadOnly)
    class USpacelProceduralMeshComponent* ShipShellComponent = nullptr;

    UPROPERTY(Category = "Movement", EditAnywhere, BlueprintReadWrite)
    float MaxForwardSpeed = 6000.0f;

    UPROPERTY(Category = "Movement", EditAnywhere, BlueprintReadWrite)
    float FlightAttitudeSpeed = 30.0f;

    UPROPERTY(Category = "Movement", EditAnywhere, BlueprintReadWrite)
    float TurnSpeed = 30.0f;

    UPROPERTY(Category = "Movement", EditAnywhere, BlueprintReadWrite)
    float UpSpeed = 40.0f;

    UPROPERTY(Category = "Camera", EditAnywhere, BlueprintReadWrite)
    float MultiplierSpringArmSize = 2.0f;

    UPROPERTY(Category = "DataAsset", EditAnywhere, BlueprintReadWrite)
    class UShipModuleDataAsset* ModuleDataAsset = nullptr;

protected:
    /* current percent speed value 0.0f - 1.0f */
    UPROPERTY(Replicated)
    float PercentSpeed = 0.0f;

    /* when flight attitude change -1.0f or 0.0f or 1.0f */
    UPROPERTY(Replicated)
    float PercentFlightAttitude = 0.0f;

    /* when turn change -1.0f or 0.0f or 1.0f */
    UPROPERTY(Replicated)
    float PercentTurn = 0.0f;

    /* when up change -1.0f or 0.0f or 1.0f */
    UPROPERTY(Replicated)
    float PercentUp = 0.0f;

private:
    /* default spring arm size */
    std::optional<float> m_springArmDefaultSize = std::nullopt;
};
