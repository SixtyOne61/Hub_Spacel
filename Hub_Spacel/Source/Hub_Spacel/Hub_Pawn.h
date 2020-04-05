// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Hub_Pawn.generated.h"

UCLASS()
class HUB_SPACEL_API AHub_Pawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AHub_Pawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;


	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	/* bind function */
	void input_Fire();
	void input_Speed(float _val);
	void input_MoveUp(float _val);
	void input_MoveRight(float _val);

    /* bind function rework */
    void input_MoveTargetUp(float _val);
    void input_MoveTargetRight(float _val);
    void input_SnapOn();
    void input_SnapOff();

	/* server function */
	UFUNCTION(Server, Reliable, WithValidation)
	void server_Fire();

    /* function call for generate our mesh */
    void generateMesh();
    void generateBase();
    void generateShell();
    void generateEngine();

    /* snap mesh to target crosshair location */
    void snapTarget(float _deltaTime);

protected:
    UPROPERTY(Category = "UI", EditAnywhere, BlueprintReadOnly)
    FVector2D CrosshairPosition = FVector2D::ZeroVector;

    UPROPERTY(Category = "Sensibility", EditAnywhere, BlueprintReadWrite)
    float SensibilityCrosshair = 0.004f;
    UPROPERTY(Category = "Sensibility", EditAnywhere, BlueprintReadWrite)
    float SensibilitySnap = 0.00001f;

    UPROPERTY(Category = "Snap", EditAnywhere, BlueprintReadWrite)
    float TimeToResetSnap = 0.6f;

    /* Procedural mesh for this ship */
    UPROPERTY(Category = "Mesh", VisibleDefaultsOnly, BlueprintReadOnly)
    class USpacelProceduralMeshComponent* ProceduralSpaceShipBase = nullptr;
    UPROPERTY(Category = "Mesh", VisibleDefaultsOnly, BlueprintReadOnly)
    class USpacelProceduralMeshComponent* ProceduralSpaceShipShell = nullptr;
    UPROPERTY(Category = "Mesh", VisibleDefaultsOnly, BlueprintReadOnly)
    class USpacelProceduralMeshComponent* ProceduralSpaceShipEngine = nullptr;

    /* Materials for ship */
    UPROPERTY(Category = "Material", EditAnywhere, BlueprintReadWrite)
    class UMaterialInstance* MatBase = nullptr;
    UPROPERTY(Category = "Material", EditAnywhere, BlueprintReadWrite)
    class UMaterialInstance* MatShell = nullptr;
    UPROPERTY(Category = "Material", EditAnywhere, BlueprintReadWrite)
    class UMaterialInstance* MatEngine = nullptr;

	/** Spring arm that will offset the camera */
	UPROPERTY(Category = "Camera", VisibleDefaultsOnly, BlueprintReadOnly)
	class USpringArmComponent* SpringArm;

	/** Camera component that will be our viewpoint */
	UPROPERTY(Category = "Camera", VisibleDefaultsOnly, BlueprintReadOnly)
	class UCameraComponent* Camera;

	UPROPERTY(Category = "Bullet", EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class ASimpleBullet> SimpleBulletClass;

private:
	// -- speed
	float m_acceleration = 250.0f;
	float m_currentForwardSpeed = 0.0f;
	const float m_minSpeed = 200.0f;
	const float m_maxSpeed = 4000.0f;

	// -- rotation common param
	const float m_interpSpeed = 2.0f;
	const float m_turnSpeed = 50.0f;

	// -- move up / pitch
	float m_currentPitchSpeed = 0.0f;

	// -- move right / yaw
	float m_currentYawSpeed = 0.0f;

	// -- move right / roll
	const float m_rollSpeed = 25.0f;
	float m_currentRollSpeed = 0.0f;

    /* true if we snap for look at of mesh */
    bool m_isSnap = false;
    /* default rotator value for snap */
    FRotator m_defaultRotation = FRotator::ZeroRotator;
    /* progress reset snap when we release snap for back to default value and set actor rotation */
    float m_progressResetSnap = 0.0f;
    /* save value when we release snap */
    FRotator m_snapRelativeRotationOnRelease = FRotator::ZeroRotator;
    FRotator m_snapRotationOnRelease = FRotator::ZeroRotator;

    /* viewport size */
    FVector2D m_viewportSize = FVector2D::ZeroVector;

public:
	// -- get
	/** Returns SpringArm subobject **/
	FORCEINLINE class USpringArmComponent* GetSpringArm() const { return SpringArm; }
	/** Returns Camera subobject **/
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }
};
