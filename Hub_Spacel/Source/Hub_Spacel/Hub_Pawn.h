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
	// -- bind function
	void input_Fire();
	void input_Speed(float _val);
	void input_MoveUp(float _val);
	void input_MoveRight(float _val);
	void input_HandBrakePress();

	// -- server function
	UFUNCTION(Server, Reliable, WithValidation)
	void server_Fire();

protected:
	UPROPERTY(Category = Mesh, VisibleDefaultsOnly, BlueprintReadOnly)
	class UStaticMeshComponent* SpaceShipMesh;

	/** Spring arm that will offset the camera */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly)
	class USpringArmComponent* SpringArm;

	/** Camera component that will be our viewpoint */
	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly)
	class UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bullet")
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
	float m_currentRollSpeed = 0.0f;

public:
	// -- get
		/** Returns PlaneMesh subobject **/
	FORCEINLINE class UStaticMeshComponent* GetPlaneMesh() const { return SpaceShipMesh; }
	/** Returns SpringArm subobject **/
	FORCEINLINE class USpringArmComponent* GetSpringArm() const { return SpringArm; }
	/** Returns Camera subobject **/
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }

};
