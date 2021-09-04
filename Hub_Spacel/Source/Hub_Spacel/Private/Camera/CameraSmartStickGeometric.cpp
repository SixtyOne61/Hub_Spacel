// All rights reserved Zubov Alexander zubetto85@gmail.com 2019

#include "CameraSmartStickGeometric.h"


// Sets default values for this component's properties
UCameraSmartStickGeometric::UCameraSmartStickGeometric()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bTickEvenWhenPaused = false;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	// --- Trace ---
	traceEnabled = true; 
	traceRadius = 20.0f; 
	traceChannel = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	traceRate = 10.0f;
	traceTau = 0.033f;
	traceInterval = 0.085f;

	// --- Smoothing ---
	displacementMax = 50.0f; 
	stiffness = 10.0f;
	damping = 0.33f;
	displacementTau = 0.5f;

	// --- Stick Rotation ---
	stickRotPermanent = true; 
	stickRotRate = 150.0f;
	stickRotTau = 0.5f;
	stickPitchLimit = 75.0f;

	// --- Camera Rotation ---
	cameraRotPermanent = true; 
	cameraRotRate = 175.0f;
	cameraRotTau = 0.5f;
	cameraPitchLimit = 360.0f;
	cameraYawLimit = 360.f;
	cameraRotOffset = FRotator(0.0f);

	// --- Stick Telescoping ---
	telescopingRate = 200.0f;
	distanceMin = 100.0f;
	distanceMax = 10000.0f;
	telescopingTau = 0.5f;

	// --- Transfering to Preset Position ---
	transferDuration = 1.0f;
}

bool UCameraSmartStickGeometric::ValidatePivots(const USceneComponent* camera, const USceneComponent* cameraTarget, const USceneComponent* restingPoint)
{
	bool isValid{ true };
	
	if (!IsValid(camera))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: The camera is not provided or not valid"), *GetNameSafe(this));
		isValid = false;
	}
	else if (camera == cameraTarget || camera == restingPoint)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: The camera should not be the same SceneComponent as the cameraTarget or restingPoint"), *GetNameSafe(this));
		isValid = false;
	}

	if (!IsValid(cameraTarget))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: The cameraTarget is not provided or not valid"), *GetNameSafe(this));
		isValid = false;
	}
	else if (cameraTarget == camera || cameraTarget == restingPoint)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: The cameraTarget should not be the same SceneComponent as the camera or restingPoint"), *GetNameSafe(this));
		isValid = false;
	}

	if (!IsValid(restingPoint))
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: The restingPoint is not provided or not valid"), *GetNameSafe(this));
		isValid = false;
	}
	else if (restingPoint == camera || restingPoint == cameraTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s: The restingPoint should not be the same SceneComponent as the camera or cameraTarget"), *GetNameSafe(this));
		isValid = false;
	}

	return isValid;
}

void UCameraSmartStickGeometric::SetComponents(USceneComponent* camera, USceneComponent* cameraTarget, USceneComponent* restingPoint)
{
	if (!ValidatePivots(camera, cameraTarget, restingPoint))
		return;

	pCamera = camera;
	pCameraTarget = cameraTarget;
	pRestingPoint = restingPoint;

	pCamera->AttachToComponent(pRestingPoint,
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, EAttachmentRule::KeepRelative, true));

	pRestingPoint->AttachToComponent(pCameraTarget,
		FAttachmentTransformRules(EAttachmentRule::KeepRelative, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, true));

	pCamera->K2_SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f), false, svcHitResult, true);
	pCameraTarget->K2_SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f), false, svcHitResult, true);

	traceIgnore.Emplace(pCameraTarget->GetOwner());

	LookAtTarget();
}

void UCameraSmartStickGeometric::CorrectRestingLocation()
{
	FVector restP{ pRestingPoint->GetRelativeLocation() };

	if (restP.Size() == 0.0f)
	{
		restP.X = -1.0f;
		restP.Y = 0.0f;
		restP.Z = 0.0f;
	}
	else
	{
		restP = UKismetMathLibrary::ClampVectorSize(restP, distanceMin, distanceMax);
	}

	// --- Adjust relative yaw and pitch of the Target to place the RestPoint on negative half of X-axis of the Target ---
	float yaw;
	float pitch;
	UKismetMathLibrary::GetYawPitchFromVector(-restP, yaw, pitch);

	pCameraTarget->K2_SetRelativeRotation(FRotator(pitch, yaw, 0.0f), false, svcHitResult, true);

	restP.X = -restP.Size();
	restP.Y = 0.0f;
	restP.Z = 0.0f;

	pRestingPoint->K2_SetRelativeLocation(restP, false, svcHitResult, true);
}

// Called when the game starts
void UCameraSmartStickGeometric::BeginPlay()
{
	Super::BeginPlay();

	if (!ValidatePivots(pCamera, pCameraTarget, pRestingPoint))
	{
		SetComponentTickEnabled(false);

		UE_LOG(LogTemp, Warning,
			TEXT("%s: Scene components for the pivots are not set properly therefore ticking of this component has been disabled"),
			*GetFullNameSafe(this));
	}

	CorrectRestingLocation();
	SetRotLimits();
	StorePosition(0);

	svcRestPo = pRestingPoint->GetComponentLocation();
}

// Called every frame
void UCameraSmartStickGeometric::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TransferRestingPoint(DeltaTime);
	MoveTowardRest(DeltaTime);
	LookAtTarget();
}

void UCameraSmartStickGeometric::LookAtTarget()
{
	float pitch{ pCameraTarget->GetRelativeRotation().Pitch };

	FRotator rotInUnpitched(pitch, 0.0f, 0.0f);
	FRotator rotToPitched(-pitch, 0.0f, 0.0f);

	FVector camInTarget{ pCamera->GetRelativeLocation() + pRestingPoint->GetRelativeLocation() };

	rotInUnpitched = UKismetMathLibrary::MakeRotFromX(rotInUnpitched.RotateVector(-camInTarget));

	rotInUnpitched.Pitch += cameraRotOffset.Pitch;
	rotInUnpitched.Yaw += cameraRotOffset.Yaw;

	rotToPitched = UKismetMathLibrary::ComposeRotators(rotInUnpitched, rotToPitched);

	pCamera->K2_SetRelativeRotation(rotToPitched, false, svcHitResult, true);
}

void UCameraSmartStickGeometric::MoveTowardRest(float dt)
{
	FTransform restTfm{ pRestingPoint->GetComponentTransform() };
	FVector restP = restTfm.GetTranslation();
	FVector restV{ (restP - svcRestPo) / dt };
	FVector accelDisplacement{ ((svcRestVo - restV) / (dt * stiffness)).GetClampedToMaxSize(displacementMax) }; //vector opposite to the acceleration of the resting point

	// update Po and Vo
	svcRestPo = restP;
	svcRestVo = restV;

	// in pRestingPoint frame
	accelDisplacement = restTfm.GetRotation().UnrotateVector(accelDisplacement);

	// applying low pass filter to update the svcStepToDisplacement:
	// Ynew = Yprev + alpha * (Xnew - Yprev)
	float alpha{ 1.0f / (1.0f + displacementTau / dt) };
	FVector step{ (accelDisplacement - svcCamFake) * (dt / damping) }; // Xnew

	svcStepToDisplacement += alpha * (step - svcStepToDisplacement);
	svcCamFake += svcStepToDisplacement;

	if (traceEnabled)
	{
		pCamera->K2_SetRelativeLocation(HitTest(dt, restTfm), false, svcHitResult, true);
	}
	else
	{
		pCamera->K2_SetRelativeLocation(svcCamFake, false, svcHitResult, true);
	}
}

FVector UCameraSmartStickGeometric::HitTest(float dt, const FTransform& restTfm)
{
	// find the new trace vector in pRestingPoint frame;
	// the following expression implies that pRestingPoint relative rotation is always ZeroRotator
	FVector targetToFake{ svcCamFake + pRestingPoint->GetRelativeLocation() };

	// shift vector from the pRestingPoint in pRestingPoint frame
	FVector hitShift;

	if (svcHitTimer > 0.0f)
	{
		svcHitTimer -= dt;

		if (svcWasHit)
		{
			// update svcDeltaToHit by orienting it along the new trace vector;
			// here we approximate the new svcDeltaToHit by the projection on the trace line
			svcDeltaToHit = svcDeltaToHit.ProjectOnTo(targetToFake);

			hitShift = svcDeltaToHit - pRestingPoint->GetRelativeLocation();
		}
		else
			hitShift = svcCamFake;
	}
	else
	{
		svcHitTimer = traceInterval;

		svcWasHit = UKismetSystemLibrary::SphereTraceSingle(
			this,
			pCameraTarget->GetComponentLocation(),
			restTfm.TransformPositionNoScale(svcCamFake),
			traceRadius,
			traceChannel,
			false,	/*traceComplex*/
			traceIgnore,
			EDrawDebugTrace::None,
			svcHitResult,
			true /*ignoreSelf*/);

		if (svcWasHit)
		{
			hitShift = restTfm.InverseTransformPositionNoScale(svcHitResult.Location);
			svcDeltaToHit = hitShift + pRestingPoint->GetRelativeLocation();
		}
		else
			hitShift = svcCamFake;
	}

	// update svcDeltaToFake by orienting it along the new trace vector;
	// here we approximate the new svcDeltaToFake by the projection on the trace line
	svcDeltaToFake = svcDeltaToFake.ProjectOnTo(targetToFake);

	// pCamera movement in pRestingPoint frame is decomposed into 
	// movement to the accelShift (presented by svcCamFake) and
	// movement along the trace line (presented by svcStepToHit);
	// camP is the pCamera position on the trace line and it is the first part of the decomposition
	FVector camP{ svcCamFake + svcDeltaToFake };

	// apply low pass filter to update the svcStepToHit
	// Ynew = Yprev + alpha * (Xnew - Yprev)
	float alpha{ 1.0f / (1.0f + traceTau / dt) };
	svcStepToHit += alpha * ((hitShift - camP) * (traceRate * dt) - svcStepToHit);

	// update svcDeltaToFake due to the camera movement along trace line
	svcDeltaToFake += svcStepToHit;

	return camP + svcStepToHit;
}

void UCameraSmartStickGeometric::SetRotLimits()
{
	stickIniRot = pCameraTarget->GetRelativeRotation();

	stickPitchMin = -stickPitchLimit - stickIniRot.Pitch;
	stickPitchMax = stickPitchLimit - stickIniRot.Pitch;
}

void UCameraSmartStickGeometric::RotateStick(float yaw, float pitch, float deltaTime)
{
	float alpha{ 1.0f / (1.0f + stickRotTau / deltaTime) };

	if (stickRotPermanent)
	{
		// --- Pitch ---
		stickPitchRate += alpha * (stickRotRate * pitch - stickPitchRate); // low-pass filtering
		stickPitch += stickPitchRate * deltaTime;
		stickPitch = UKismetMathLibrary::FClamp(stickPitch, stickPitchMin, stickPitchMax);

		// --- Yaw ---
		stickYawRate += alpha * (stickRotRate * yaw - stickYawRate);
		stickYaw += stickYawRate * deltaTime;
		stickYaw = UXMath::inlNormalizeClampedAngleD(stickYaw);
	}
	else
	{
		// --- Pitch ---
		pitch = UKismetMathLibrary::FClamp(90.0f * pitch, stickPitchMin, stickPitchMax);
		stickPitch += alpha * (pitch - stickPitch); // low-pass filtering

		// --- Yaw ---
		yaw *= 180.0f;
		stickYaw += alpha * (yaw - stickYaw); // low-pass filtering
	}

	pCameraTarget->K2_SetRelativeRotation(FRotator(stickPitch + stickIniRot.Pitch, stickYaw + stickIniRot.Yaw, 0.0f), false, svcHitResult, true);
}

void UCameraSmartStickGeometric::RotateCamera(float yaw, float pitch, float deltaTime)
{
	if (cameraRotPermanent)
	{
		float da{ cameraRotRate * deltaTime };

		if (!UXMath::inlClampf(cameraRotOffset.Yaw + yaw * da, cameraYawLimit, cameraRotOffset.Yaw))
		{
			cameraRotOffset.Yaw = UXMath::inlNormalizeClampedAngleD(cameraRotOffset.Yaw);
		}

		if (!UXMath::inlClampf(cameraRotOffset.Pitch + pitch * da, cameraPitchLimit, cameraRotOffset.Pitch))
		{
			cameraRotOffset.Pitch = UXMath::inlNormalizeClampedAngleD(cameraRotOffset.Pitch);
		}
	}
	else
	{
		float alpha{ 1.0f / (1.0f + cameraRotTau / deltaTime) };

		yaw *= 180.0f;
		pitch *= 180.0f;

		cameraRotOffset.Yaw += alpha * (UXMath::inlClampf(yaw, cameraYawLimit) - cameraRotOffset.Yaw);
		cameraRotOffset.Pitch += alpha * (UXMath::inlClampf(pitch, cameraPitchLimit) - cameraRotOffset.Pitch);
	}
}

void UCameraSmartStickGeometric::ChangeDistance(float input, float deltaTime)
{
	float alpha{ 1.0f / (1.0f + telescopingTau / deltaTime) };
	telescopingRateCurrent += alpha * (telescopingRate * input - telescopingRateCurrent); // low-pass filtering

	float x{ pRestingPoint->GetRelativeLocation().X };
	x += telescopingRateCurrent * deltaTime;
	x = UKismetMathLibrary::FClamp(abs(x), distanceMin, distanceMax);

	pRestingPoint->K2_SetRelativeLocation(FVector(-x, 0.0f, 0.0f), false, svcHitResult, true);
}

void UCameraSmartStickGeometric::AxesPoolInput(FAxesPool input)
{
	if (!isTransferred)
	{
		ChangeDistance(input.Distance, input.DeltaTime);
		RotateStick(input.StickYaw, input.StickPitch, input.DeltaTime);
	}

	RotateCamera(input.CameraYaw, input.CameraPitch, input.DeltaTime);
}

void UCameraSmartStickGeometric::StorePosition(int32 index)
{
	presetPositions.Emplace(index, FDistanceRotator{ pRestingPoint->GetRelativeLocation().X , pCameraTarget->GetRelativeRotation() });
}

bool UCameraSmartStickGeometric::TransferToPreset(int32 index)
{
	if (presetPositions.Contains(index))
	{
		selectedPresetPosID = index;

		trfXa = pRestingPoint->GetRelativeLocation().X;
		trfXb = presetPositions[index].Distance;

		trfRotA = pCameraTarget->GetRelativeRotation();
		trfRotBnorm = presetPositions[index].Rotation;

		trfRotB.Pitch = trfRotBnorm.Pitch;
		trfRotB.Yaw = UXMath::inlNearestAngle(trfRotA.Yaw, trfRotBnorm.Yaw);

		trfTimer = 0.0f;
		isTransferred = true;

		return true;
	}
	else
		return false;
}

void UCameraSmartStickGeometric::TransferRestingPoint(float deltaTime)
{
	if (isTransferred)
	{
		trfTimer += deltaTime;

		if (trfTimer < transferDuration)
		{
			float alpha{ UKismetMathLibrary::FInterpEaseInOut(0.0f, 1.0f, trfTimer / transferDuration, 2.0f) };

			FRotator rot{ UKismetMathLibrary::RLerp(trfRotA, trfRotB, alpha, false) };
			pCameraTarget->K2_SetRelativeRotation(rot, false, svcHitResult, true);

			float X{ UKismetMathLibrary::Lerp(trfXa, trfXb, alpha) };
			pRestingPoint->K2_SetRelativeLocation(FVector(X, 0.0f, 0.0f), false, svcHitResult, true);
		}
		else
		{
			pCameraTarget->K2_SetRelativeRotation(trfRotBnorm, false, svcHitResult, true);
			pRestingPoint->K2_SetRelativeLocation(FVector(trfXb, 0.0f, 0.0f), false, svcHitResult, true);

			SetRotLimits();
			stickPitch = 0.0f;
			stickYaw = 0.0f;

			isTransferred = false;
		}
	}
}

// ---------------------------------------------------------------------------------------------------------------------
// --- BlueprintSetters ------------------------------------------------------------------------------------------------
void UCameraSmartStickGeometric::displacementMax_BPSet(float value) { SetDisplacementMax(value); }
void UCameraSmartStickGeometric::stiffness_BPSet(float value) { SetStiffness(value); }
void UCameraSmartStickGeometric::damping_BPSet(float value) { SetDamping(value); }
void UCameraSmartStickGeometric::displacementTau_BPSet(float value) { SetDisplacementTau(value); }

void UCameraSmartStickGeometric::traceRadius_BPSet(float value) { SetTraceRadius(value); }
void UCameraSmartStickGeometric::traceRate_BPSet(float value) { SetTraceRate(value); }
void UCameraSmartStickGeometric::traceTau_BPSet(float value) { SetTraceTau(value); }
void UCameraSmartStickGeometric::traceInterval_BPSet(float value) { SetTraceInterval(value); }

void UCameraSmartStickGeometric::stickRotPermanent_BPSet(bool value) { SetStickRotPermanent(value); }
void UCameraSmartStickGeometric::stickRotRate_BPSet(float value) { SetStickRotRate(value); }
void UCameraSmartStickGeometric::stickRotTau_BPSet(float value) { SetStickRotTau(value); }
void UCameraSmartStickGeometric::stickPitchLimit_BPSet(float value) { SetStickPitchLimit(value); }

void UCameraSmartStickGeometric::cameraRotRate_BPSet(float value) { SetCameraRotRate(value); }
void UCameraSmartStickGeometric::cameraRotTau_BPSet(float value) { SetCameraRotTau(value); }

void UCameraSmartStickGeometric::telescopingRate_BPSet(float value) { SetTelescopingRate(value); }
void UCameraSmartStickGeometric::telescopingTau_BPSet(float value) { SetTelescopingTau(value); }
void UCameraSmartStickGeometric::distanceMin_BPSet(float value) { SetDistanceMin(value); }
void UCameraSmartStickGeometric::distanceMax_BPSet(float value) { SetDistanceMax(value); }
