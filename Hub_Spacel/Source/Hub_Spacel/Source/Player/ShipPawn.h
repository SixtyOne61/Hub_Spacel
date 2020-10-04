// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ShipPawn.generated.h"

UCLASS()
class HUB_SPACEL_API AShipPawn : public APawn
{
	GENERATED_BODY()

    friend class APlayerShipController;

public:
	// Sets default values for this pawn's properties
	AShipPawn();

public:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

    // Called when something hit pawn
    virtual void NotifyHit(class UPrimitiveComponent* _myComp, class AActor* _other, class UPrimitiveComponent* _otherComp, bool _bSelfMoved, FVector _hitLocation, FVector _hitNormal, FVector _normalImpulse, const FHitResult& _hit) override;

private:
    /* build ship with all module */
    void buildShip();

    /* move ship */
    void move();

public:
    UPROPERTY(Category = "Ship", VisibleDefaultsOnly, BlueprintReadOnly)
    class USpacelProceduralMeshComponent* ShipBaseComponent = nullptr;

    UPROPERTY(Category = "Ship", VisibleDefaultsOnly, BlueprintReadOnly)
    class USpacelProceduralMeshComponent* ShipEngineComponent = nullptr;

    UPROPERTY(Category = "Component", VisibleDefaultsOnly, BlueprintReadOnly)
    class USpringArmComponent* SpringArmComponent;

    UPROPERTY(Category = "Component", VisibleDefaultsOnly, BlueprintReadOnly)
    class UCameraComponent* CameraComponent;

    UPROPERTY(Category = "Movement", VisibleDefaultsOnly, BlueprintReadOnly)
    class UShipPawnMovement* ShipPawnMovement = nullptr;

    UPROPERTY(Category = "Movement", EditAnywhere, BlueprintReadWrite)
    float MaxForwardSpeed = 6000.0f;

    UPROPERTY(Category = "Material", EditAnywhere, BlueprintReadWrite)
    class UMaterialInstance* MatBase = nullptr;

protected:
    /* current percent speed value 0.0f - 1.0f */
    UPROPERTY(Replicated)
    float PercentSpeed = 0.0f;
};
