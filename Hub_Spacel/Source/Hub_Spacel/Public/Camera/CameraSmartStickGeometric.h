// All rights reserved Zubov Alexander zubetto85@gmail.com 2019

#pragma once

#include "CoreMinimal.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "XMath.h"
#include "AxesPool.h"
#include "Components/ActorComponent.h"
#include "CameraSmartStickGeometric.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable )
class HUB_SPACEL_API UCameraSmartStickGeometric : public UActorComponent
{
	GENERATED_BODY()

#pragma region PUBLIC
public:
	// Sets default values for this component's properties
	UCameraSmartStickGeometric();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/*
		Sets provided scene components in the following hierarchy (parent -> child): CameraTarget -> RestingPoint -> Camera.
		Typically CameraTarget should be just a scene component attached to the actual target due to the fact
		that the Camera rotation around the target is achived by corresponding rotation of the CameraTarget
		(in this sense CameraTarget can also be thought as pivot point of an imaginary stick held the Camera).
		RestingPoint - scene component designating Camera relative location when CameraTarget velocity vector is constant
		(stick tip the Camera is attached to by damped spring)
	*/
	UFUNCTION(BlueprintCallable, Category = "Construction")
	void SetComponents(USceneComponent* camera, USceneComponent* cameraTarget, USceneComponent* restingPoint);


	//--------------------------------------------------------------------------------------------------------------------------
	// --- Smoothing options ---------------------------------------------------------------------------------------------------
	FORCEINLINE float GetDisplacementMax() { return displacementMax; }
	FORCEINLINE void SetDisplacementMax(float value) { displacementMax = value < 0.0f ? 0.0f : value; }

	FORCEINLINE float GetStiffness() { return stiffness; }
	FORCEINLINE void SetStiffness(float value) { stiffness = value <= 0.0f ? 0.000001f : value; }

	FORCEINLINE float GetDamping() { return damping; }
	FORCEINLINE void SetDamping(float value) { damping = value <= 0.0f ? 0.000001f : value; }

	FORCEINLINE float GetDisplacementTau() { return displacementTau; }
	FORCEINLINE void SetDisplacementTau(float value) { displacementTau = value < 0.0f ? 0.0f : value; }


	// -------------------------------------------------------------------------------------------------------------------------------
	// --- Obstacle Detection --------------------------------------------------------------------------------------------------------
	/*
		If true, periodic trace from the CameraTarget to the Camera is performed to detect and avoid obstacles in the path of a trace line
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Detection")
	bool traceEnabled;

	FORCEINLINE float GetTraceRadius() { return traceRadius; }
	FORCEINLINE void SetTraceRadius(float value) { traceRadius = value < 0.0f ? 0.0f : value; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Detection")
	TEnumAsByte<ETraceTypeQuery> traceChannel;

	FORCEINLINE float GetTraceRate() { return traceRate; }
	FORCEINLINE void SetTraceRate(float value) { traceRate = value < 0.0f ? 0.0f : value; }

	FORCEINLINE float GetTraceTau() { return traceTau; }
	FORCEINLINE void SetTraceTau(float value) { traceTau = value < 0.0f ? 0.0f : value; }

	FORCEINLINE float GetTraceInterval() { return traceInterval; }
	FORCEINLINE void SetTraceInterval(float value) { traceInterval = value < 0.0f ? 0.0f : value; }

	/*
		The array of actors to be ignored by the trace.
		The actor owning this component is added to the array in the SetComponents method
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Detection")
	TArray<AActor*> traceIgnore;


	// ------------------------------------------------------------------------------------------------------------
	// --- Axes Input ---------------------------------------------------------------------------------------------
	/*
		Sets position of the RestingPoint and orientation of the Camera accordingly to the provided axes values.
		If RestingPoint is currently being transferred to one of the preset position
		then only camera rotation-input will have effect
	*/
	UFUNCTION(BlueprintCallable, Category = "Axes Input")
	void AxesPoolInput(FAxesPool input);


	// ---------------------------------------------------------------------------------------------
	// --- Axes Input | Stick Rotation -------------------------------------------------------------
	FORCEINLINE bool GetStickRotPermanent() { return stickRotPermanent; }
	FORCEINLINE void SetStickRotPermanent(bool value)
	{
		if (value != stickRotPermanent)
		{
			stickRotPermanent = value;

			if (!value)
			{
				stickPitchRate = 0.0f;
				stickYawRate = 0.0f;
			}
		}
	}

	FORCEINLINE float GetStickRotRate() { return stickRotRate; }
	FORCEINLINE void SetStickRotRate(float value) { stickRotRate = value < 0.0f ? 0.0f : (value > 360.0f ? 360.0f : value); }

	FORCEINLINE float GetStickRotTau() { return stickRotTau; }
	FORCEINLINE void SetStickRotTau(float value) { stickRotTau = value < 0.0f ? 0.0f : value; }

	FORCEINLINE float GetStickPitchLimit() { return stickPitchLimit; }
	FORCEINLINE void SetStickPitchLimit(float value) 
	{
		stickPitchLimit = value < 0.0f ? 0.0f : (value > 90.0f ? 90.0f : value); 
		SetRotLimits();
	}

	/*
		Rotates imaginary stick causing changing of the RestingPoint relative location.
		The yaw and pitch are axes input values. The axis scale of 1.0 is expected for each input.
		Does not check if RestingPoint is being transferred now to one of the preset position
	*/
	UFUNCTION(BlueprintCallable, Category = "Axes Input|Stick Rotation")
	void RotateStick(float yaw, float pitch, float deltaTime);


	// --------------------------------------------------------------------------------------------
	// --- Axes Input | Camera Rotation -----------------------------------------------------------
	/*
		Angular offset in degrees from the gaze direction aimed at the CameraTarget.
		The roll-component does not affect Camera rotation.
	*/
	UPROPERTY(BlueprintReadWrite, Transient, Category = "Axes Input|Camera Rotation")
	FRotator cameraRotOffset;

	/*
		If true then the Camera retains its current rotation (i.e. the CameraRotOffset retains its current value) after axes input values become zero,
		otherwise, the Camera is aimed at the CameraTarget (i.e. the CameraRotOffset becomes zero) after axes input values become zero.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axes Input|Camera Rotation")
	bool cameraRotPermanent;

	FORCEINLINE float GetCameraRotRate() { return cameraRotRate; }
	FORCEINLINE void SetCameraRotRate(float value) { cameraRotRate = value < 0.0f ? 0.0f : (value > 180.0f ? 180.0f : value); }

	FORCEINLINE float GetCameraRotTau() { return cameraRotTau; }
	FORCEINLINE void SetCameraRotTau(float value) { cameraRotTau = value < 0.0f ? 0.0f : value; }

	FORCEINLINE float GetCameraPitchLimit() { return cameraPitchLimit; }
	FORCEINLINE void SetCameraPitchLimit(float value) { cameraPitchLimit = fabsf(value); }

	FORCEINLINE float GetCameraYawLimit() { return cameraYawLimit; }
	FORCEINLINE void SetCameraYawLimit(float value) { cameraYawLimit = fabsf(value); }

	/*
		The yaw and pitch are axes input values. The axis scale of 1.0 is expected for each input
	*/
	UFUNCTION(BlueprintCallable, Category = "Axes Input|Camera Rotation")
	void RotateCamera(float yaw, float pitch, float deltaTime);


	// ----------------------------------------------------------------------------------------------
	// --- Axes Input | Stick Telescoping -----------------------------------------------------------
	FORCEINLINE float GetTelescopingRate() { return telescopingRate; }
	FORCEINLINE void SetTelescopingRate(float value) { telescopingRate = value < 0.0f ? 0.0f : value; }

	FORCEINLINE float GetTelescopingTau() { return telescopingTau; }
	FORCEINLINE void SetTelescopingTau(float value) { telescopingTau = value < 0.0f ? 0.0f : value; }

	FORCEINLINE float GetDistanceMin() { return distanceMin; }
	FORCEINLINE void SetDistanceMin(float value) { distanceMin = value < 0.0f ? 0.0f : (value > distanceMax ? distanceMax : value); }

	FORCEINLINE float GetDistanceMax() { return distanceMax; }
	FORCEINLINE void SetDistanceMax(float value) { distanceMax = value < distanceMin ? distanceMin : value; }

	/*
		Moves the RestingPoint toward the CameraTarget if input is positive and
		away from the CameraTarget if input is negative.
		Does not check if RestingPoint is being transferred now to one of the preset position
	*/
	UFUNCTION(BlueprintCallable, Category = "Axes Input|Stick Telescoping")
	void ChangeDistance(float input, float deltaTime);


	// -------------------------------------------------------------------------------------------------------
	// --- Preset Positions ----------------------------------------------------------------------------------
	/*
		Duration in sec of the RestingPoint transfer to one of the preset position
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset Positions", meta = (ClampMin = "0.0"))
	float transferDuration;

	/*
		Stores current relative position of the RestingPoint into the array to the specified index.
		Position previously stored under the same index will be rewritten by the current one.
		This method is called in the BeginPlay event of this component with index equal zero
		thereby storing initial position to the zero index
	*/
	UFUNCTION(BlueprintCallable, Category = "Preset Positions")
	void StorePosition(int32 index);

	/*
		If the PresetPositions contains an entry at the specified index then RestingPoint transfer begins 
		and this function returns true, otherwise returns false
	*/
	UFUNCTION(BlueprintCallable, Category = "Preset Positions")
	UPARAM(DisplayName = "Started") bool TransferToPreset(int32 index);

	/*
		Stores preset positions of the RestingPoint:
			FDistanceRotator.Distance is relative X-coordinate of the RestingPoint;
			FDistanceRotator.Rotation is relative rotation of the CameraTarget
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preset Positions")
	TMap<int32, FDistanceRotator> presetPositions;

	FORCEINLINE int GetSelectedPresetPositionID() { return selectedPresetPosID; }
#pragma endregion


#pragma region PROTECTED
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// --- Pivots ----------------------------------------------------------------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Construction")
	USceneComponent* pCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Construction")
	USceneComponent* pCameraTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Construction")
	USceneComponent* pRestingPoint;

	bool ValidatePivots(const USceneComponent* camera, const USceneComponent* cameraTarget, const USceneComponent* restingPoint);


	//--------------------------------------------------------------------------------------------------------------------------
	// --- Smoothing options ---------------------------------------------------------------------------------------------------
	/*
		The displacement limit of the Camera from the RestingPoint.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing", meta = (ClampMin = "0.0"), BlueprintSetter = displacementMax_BPSet)
	float displacementMax;

	UFUNCTION(BlueprintSetter)
	void displacementMax_BPSet(float value);

	/*
		Stiffness of an imaginary spring pulling the Camera to the RestingPoint.
		The lower the value, the greater displacement of the Camera from the RestingPoint when the CameraTarget moves with acceleration
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing", meta = (ClampMin = "0.000001"), BlueprintSetter = stiffness_BPSet)
	float stiffness;

	UFUNCTION(BlueprintSetter)
	void stiffness_BPSet(float value);

	/*
		Damping coefficient of an imaginary spring pulling the Camera to the RestingPoint.
		The lower the value, the faster the Camera responds to changes in CameraTarget acceleration.
		Too low values can cause abrupt Camera movements. In such cases, try increasing DisplacementTau.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing", meta = (ClampMin = "0.000001"), BlueprintSetter = damping_BPSet)
	float damping;

	UFUNCTION(BlueprintSetter)
	void damping_BPSet(float value);

	/*
		Time constant in sec of the low-pass filter applied to Camera movements caused by accelerated motion of the CameraTarget.
		Camera reaches new position corresponding to current acceleration in about three Tau.
		Typically you start from choosing of the Stiffness and Damping with DisplacementTau close to zero and
		increase DisplacementTau in cases of abrupt Camera movements.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Smoothing", meta = (ClampMin = "0.0"), BlueprintSetter = displacementTau_BPSet)
	float displacementTau;

	UFUNCTION(BlueprintSetter)
	void displacementTau_BPSet(float value);


	// -------------------------------------------------------------------------------------------------------------------------------
	// --- Obstacle Detection --------------------------------------------------------------------------------------------------------
	/*
		Radius of the sphere used for the tracing
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Detection", meta = (ClampMin = "0.0"), BlueprintSetter = traceRadius_BPSet)
	float traceRadius;

	UFUNCTION(BlueprintSetter)
	void traceRadius_BPSet(float value);

	/*
		Maximum movement speed in cm per sec of the Camera along a tracing line
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Detection", meta = (ClampMin = "0.0"), BlueprintSetter = traceRate_BPSet)
	float traceRate;

	UFUNCTION(BlueprintSetter)
	void traceRate_BPSet(float value);

	/*
		Time constant in sec of the low-pass filter applied to the Camera movement along a tracing line.
		Camera reaches the maximum movement speed along a tracing line in about three Tau.
		The lower the value, the faster response to obstacles located in the path of a tracing line.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Detection", meta = (ClampMin = "0.0"), BlueprintSetter = traceTau_BPSet)
	float traceTau;

	UFUNCTION(BlueprintSetter)
	void traceTau_BPSet(float value);

	/*
		The time in seconds between two consecutive traces. If equal zero then trace will be performed every frame.
		If jerky movement of the camera occurs with TraceInterval values greater than zero try increasing the TraceTau
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Obstacle Detection", meta = (ClampMin = "0.0"), BlueprintSetter = traceInterval_BPSet)
	float traceInterval;

	UFUNCTION(BlueprintSetter)
	void traceInterval_BPSet(float value);


	// ---------------------------------------------------------------------------------------------
	// --- Axes Input | Stick Rotation -------------------------------------------------------------
	/*
		If true then the stick (i.e. the CameraTarget) retains its current rotation after axes input values become zero,
		otherwise, stick rotation returns to selected preset rotation after axes input values become zero
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axes Input|Stick Rotation", BlueprintSetter = stickRotPermanent_BPSet)
	bool stickRotPermanent;

	UFUNCTION(BlueprintSetter)
	void stickRotPermanent_BPSet(bool value);

	/*
		In degrees per sec
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axes Input|Stick Rotation", meta = (ClampMin = "0.0", ClampMax = "360.0"), BlueprintSetter = stickRotRate_BPSet)
	float stickRotRate;

	UFUNCTION(BlueprintSetter)
	void stickRotRate_BPSet(float value);

	/*
		Time constant in sec of the low-pass filter applied to axes input.
		The higher the value, the smoother the rotation.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axes Input|Stick Rotation", meta = (ClampMin = "0.0"), BlueprintSetter = stickRotTau_BPSet)
	float stickRotTau;

	UFUNCTION(BlueprintSetter)
	void stickRotTau_BPSet(float value);

	/*
		In degrees
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axes Input|Stick Rotation", meta = (ClampMin = "0.0", ClampMax = "90"), BlueprintSetter = stickPitchLimit_BPSet)
	float stickPitchLimit;

	UFUNCTION(BlueprintSetter)
	void stickPitchLimit_BPSet(float value);


	// --------------------------------------------------------------------------------------------
	// --- Axes Input | Camera Rotation -----------------------------------------------------------
	/*
		In degrees per sec
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axes Input|Camera Rotation", meta = (ClampMin = "0.0", ClampMax = "180.0"), BlueprintSetter = cameraRotRate_BPSet)
	float cameraRotRate;

	UFUNCTION(BlueprintSetter)
	void cameraRotRate_BPSet(float value);

	/*
		Time constant in sec of the low-pass filter applied to axes input.
		The higher the value, the smoother the rotation.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axes Input|Camera Rotation", meta = (ClampMin = "0.0"), BlueprintSetter = cameraRotTau_BPSet)
	float cameraRotTau;

	UFUNCTION(BlueprintSetter)
	void cameraRotTau_BPSet(float value);

	/*
		Absolute limit in degrees. Values greater than 180.0 set unlimited pitch change
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axes Input|Camera Rotation", meta = (ClampMin = "0.0"))
	float cameraPitchLimit;

	/*
		Absolute limit in degrees. Values greater than 180.0 set unlimited yaw change
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axes Input|Camera Rotation", meta = (ClampMin = "0.0"))
	float cameraYawLimit;


	// ----------------------------------------------------------------------------------------------
	// --- Axes Input | Stick Telescoping -----------------------------------------------------------
	/*
		In cm per sec
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axes Input|Stick Telescoping", meta = (ClampMin = "0.0"), BlueprintSetter = telescopingRate_BPSet)
	float telescopingRate;

	UFUNCTION(BlueprintSetter)
	void telescopingRate_BPSet(float value);

	/*
		Time constant in sec of the low-pass filter applied to the stick telescoping.
		The higher the value, the smoother the stick telescoping.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axes Input|Stick Telescoping", meta = (ClampMin = "0.0"), BlueprintSetter = telescopingTau_BPSet)
	float telescopingTau;

	UFUNCTION(BlueprintSetter)
	void telescopingTau_BPSet(float value);

	/*
		Minimum distance from the CameraTarget at which the RestingPoint can be placed during stick telescoping
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axes Input|Stick Telescoping", meta = (ClampMin = "0.0"), BlueprintSetter = distanceMin_BPSet)
	float distanceMin;

	UFUNCTION(BlueprintSetter)
	void distanceMin_BPSet(float value);

	/*
		Maximum distance from the CameraTarget at which the RestingPoint can be placed during stick telescoping
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Axes Input|Stick Telescoping", meta = (ClampMin = "0.0"), BlueprintSetter = distanceMax_BPSet)
	float distanceMax;

	UFUNCTION(BlueprintSetter)
	void distanceMax_BPSet(float value);


	// ------------------------------------------------------------------------------------------------------
	// --- Preset Positions ---------------------------------------------------------------------------------
	UPROPERTY(BlueprintReadOnly, Transient, Category = "Preset Positions")
	int selectedPresetPosID;
#pragma endregion


#pragma region PRIVATE
private:
	// --- Service variables ---------------------------------------------------------------------------------
	FHitResult svcHitResult; // tmp

	/*
		Camera movement to the rest
			svcRestPo, svcRestVo      - in world frame
			svcStepToRest, svcCamFake - in pRestingPoint frame
	*/
	FVector svcRestPo = FVector(0.0f); 
	FVector svcRestVo = FVector(0.0f);
	FVector svcCamFake = FVector(0.0f);
	FVector svcStepToDisplacement = FVector(0.0f); 

	/*
		Obstacle Detection
		All vectors in pRestingPoint frame
			svcDeltaToHit  - from pCameraTarget to hit location
			svcDeltaToFake - from svcCamFake to pCamera
	*/
	FVector svcDeltaToHit = FVector(0.0f);
	FVector svcDeltaToFake = FVector(0.0f); 
	FVector svcStepToHit = FVector(0.0f);
	bool svcWasHit = false;
	float svcHitTimer = 0.0f;

	/*
		Stick Rotation in degrees
	*/
	float stickPitchMin = 0.0f, stickPitchMax = 0.0f, stickPitch = 0.0f, stickYaw = 0.0f, stickPitchRate = 0.0f, stickYawRate = 0.0f;
	FRotator stickIniRot = FRotator(0.0f);

	/*
		Stick Telescoping
	*/
	float telescopingRateCurrent = 0.0f;

	/*
		Preset positions
	*/
	bool isTransferred = false;
	float trfTimer = 0.0f, trfXa = 0.0f, trfXb = 0.0f;
	FRotator trfRotA = FRotator(0.0f);
	FRotator trfRotB = FRotator(0.0f); 
	FRotator trfRotBnorm = FRotator(0.0f);
	


	// --- Service methods ---------------------------------------------------------------------------------
	void LookAtTarget();

	void MoveTowardRest(float dt);

	FVector HitTest(float dt, const FTransform& restTfm);

	/*
		Serves to avoid Camera locations in nadir and zenith relative to the CameraTarget
		and also adjusts RestingPoint relative location and CameraTarget relative rotation to place
		the Camera on the negative half of X-axis of the CameraTarget.
	*/
	void CorrectRestingLocation();

	void SetRotLimits();

	void TransferRestingPoint(float deltaTime);
#pragma endregion
};
