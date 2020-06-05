// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Hub_Enum.h"
#include "Hub_Pawn.generated.h"

static int32 DebugDrawSpawnBullet = 0;

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
	virtual void Tick(float _deltaTime) override;
	virtual void NotifyHit(class UPrimitiveComponent* _myComp, class AActor* _other, class UPrimitiveComponent* _otherComp, bool _bSelfMoved, FVector _hitLocation, FVector _hitNormal, FVector _normalImpulse, const FHitResult& _hit) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* _playerInputComponent) override;

    /* for setup all module of ship */
    UFUNCTION(BlueprintCallable)
    void SetupModule(TSubclassOf<ADefaultSubMachine> _subMachine, TSubclassOf<ADefaultShell> _shell, TSubclassOf<ADefaultEngine> _engine);

protected:
	/* bind function */
	void input_FireOn();
    void input_FireOff();
	void input_Speed(float _val);
	void input_MoveUp(float _val);
	void input_MoveRight(float _val);

    /* bind function rework */
    void input_MoveTargetUp(float _val);
    void input_MoveTargetRight(float _val);
    void input_SnapOn();
    void input_SnapOff();

	/* TO DO network */
    void fireLaser(float _deltaTime);

    /* function call for generate our mesh */
    void generateMesh();

    /* init module of ship */
    template<class T>
    void initModule(UChildActorComponent*& _child, TArray<FVector> const& _ignoreCoord);

    /* snap mesh to target crosshair location */
    void snapTarget(float _deltaTime);

    /* reset crosshair position to center screen */
    void resetCrosshair();

    /* init all module */
    void initMeshModules();

protected:
    UPROPERTY(Category = "UI", EditAnywhere, BlueprintReadOnly)
    FVector2D CrosshairPosition = FVector2D::ZeroVector;

    UPROPERTY(Category = "Sensibility", EditAnywhere, BlueprintReadWrite)
    float SensibilityCrosshair = 0.004f;
    UPROPERTY(Category = "Sensibility", EditAnywhere, BlueprintReadWrite)
    float SensibilitySnap = 0.00001f;

    UPROPERTY(Category = "Snap", EditAnywhere, BlueprintReadWrite)
    float TimeToResetSnap = 0.6f;

    UPROPERTY(Category = "Controller", EditAnywhere, BlueprintReadWrite)
    float MaxForwardSpeed = 6000.0f;
    UPROPERTY(Category = "Controller", EditAnywhere, BlueprintReadWrite)
    float TurnSpeed = 50.0f;
    UPROPERTY(Category = "Controller", EditAnywhere, BlueprintReadWrite)
    float InterpSpeed = 2.0f;
    UPROPERTY(Category = "Controller", VisibleAnywhere, BlueprintReadOnly)
    float PercentSpeed = 0.0f;

    /* Procedural mesh for this ship */
    UPROPERTY(Category = "Mesh", VisibleDefaultsOnly, BlueprintReadOnly)
    class USpacelProceduralMeshComponent* ProceduralSpaceShipBase = nullptr;

    /* Materials for ship */
    UPROPERTY(Category = "Material", EditAnywhere, BlueprintReadWrite)
    class UMaterialInstance* MatBase = nullptr;

	/* Spring arm that will offset the camera */
	UPROPERTY(Category = "Camera", VisibleDefaultsOnly, BlueprintReadOnly)
	class USpringArmComponent* SpringArm;
	/* Camera component that will be our viewpoint */
	UPROPERTY(Category = "Camera", VisibleDefaultsOnly, BlueprintReadOnly)
	class UCameraComponent* Camera;
    /* Multiplier for spring arm size */
    UPROPERTY(Category = "Camera", EditAnywhere, BlueprintReadWrite)
    float MultiplierSpringArmSize = 2.0f;
    /* Field of view when focus */
    UPROPERTY(Category = "Camera", EditAnywhere, BlueprintReadWrite)
    float FieldOfViewOnFocus = 60.0f;

	UPROPERTY(Category = "Bullet", EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class AActor> LaserClass;
    UPROPERTY(Category = "Bullet", EditAnywhere, BlueprintReadOnly)
    float TimeBetweenLaserShot = 0.3f;
    
    UPROPERTY(Category = "Module", VisibleAnywhere, BlueprintReadOnly)
    class UChildActorComponent* SubMachineModule = nullptr;
    UPROPERTY(Category = "Module", VisibleAnywhere, BlueprintReadOnly)
    class UChildActorComponent* ShellModule = nullptr;
    UPROPERTY(Category = "Module", VisibleAnywhere, BlueprintReadOnly)
    class UChildActorComponent* EngineModule = nullptr;

private:
	/* speed */
	float m_currentForwardSpeed = 0.0f;
    
	/* move up / pitch */
	float m_currentPitchSpeed = 0.0f;
	/* move right / yaw */
	float m_currentYawSpeed = 0.0f;
	/* move right / roll */
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

    /* true if we hold fire input */
    bool m_isFire = false;

    /* count down between fire */
    float m_laserCountDown = 0.0f;

    /* default spring arm size */
    float m_springArmDefaultSize = 0.0f;
    /* default field of view */
    float m_fieldOfViewDefault = 0.0f;
    /* time to update field of view */
    float m_timeToUpdateFieldOfView = 0.0f;

public:
	// -- get
	/** Returns SpringArm subobject **/
	FORCEINLINE class USpringArmComponent* GetSpringArm() const { return SpringArm; }
	/** Returns Camera subobject **/
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }
};

template<class T>
void AHub_Pawn::initModule(UChildActorComponent*& _child, TArray<FVector> const& _ignoreCoord)
{
    if (!_child)
    {
        return;
    }

    T* module = Cast<T>(_child->GetChildActor());
    module->GenerateMesh(_ignoreCoord);
}
