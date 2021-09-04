// All rights reserved Zubov Alexander zubetto85@gmail.com 2019


#include "CameraSmartStickPhysical.h"

// Sets default values for this component's properties
UCameraSmartStickPhysical::UCameraSmartStickPhysical()
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
	dampingRatio = 1.0f;
	accelerationMax = 100000.0f;
	displacementMargin = 100.0f;
	stiffnessGain = 100.0f;

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

bool UCameraSmartStickPhysical::ValidatePivots(const USceneComponent* camera, const USceneComponent* cameraTarget, const USceneComponent* restingPoint)
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

void UCameraSmartStickPhysical::SetComponents(USceneComponent* camera, USceneComponent* cameraTarget, USceneComponent* restingPoint)
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

void UCameraSmartStickPhysical::CorrectRestingLocation()
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
void UCameraSmartStickPhysical::BeginPlay()
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

	inlAdjustStickRotRateFadeWidth();
	inlAdjustTelescopingFadeWidth();

	svcRestPo = pRestingPoint->GetComponentLocation();
}

// Called every frame
void UCameraSmartStickPhysical::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TransferRestingPoint(DeltaTime);
	MoveTowardRest(DeltaTime);
	LookAtTarget();
}

void UCameraSmartStickPhysical::LookAtTarget()
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

void UCameraSmartStickPhysical::MoveTowardRest(float dt)
{
	FTransform restTfm{ pRestingPoint->GetComponentTransform() };
	FVector restP = restTfm.GetTranslation();
	FVector restV{ (restP - svcRestPo) / dt };
	FVector restLocalA{ ((svcRestVo - restV) / dt).GetClampedToMaxSize(accelerationMax) }; //vector opposite to the acceleration of the resting point

	// update Po and Vo
	svcRestPo = restP;
	svcRestVo = restV;

	// in pRestingPoint frame
	restLocalA = restTfm.GetRotation().UnrotateVector(restLocalA);

	float actualStiffness{ stiffness };
	float actualDampingRatio{ dampingRatio };
	float dmax{ displacementMax + displacementMargin };

	if (svcCamFake.SizeSquared() > dmax * dmax)
	{
		actualStiffness += stiffnessGain;
		actualDampingRatio = 1.0f;
	}

	actualStiffness = fmaxf(actualStiffness, restLocalA.Size() / displacementMax);

	// --- The mass of the Camera is taken equal to 1 kg ---
	float actualDamping{ actualDampingRatio * 2.0f * sqrtf(actualStiffness) }; // DmpRatio * 2*sqrt(k*m)

	FVector camFakeA{ restLocalA - actualDamping * svcCamFakeV - actualStiffness * svcCamFake }; // m*der2(x) = F_inertia - c*der(x) - k*x
	UXMath::inlStepWithConstAcceleration(camFakeA, svcCamFakeV, svcCamFake, dt);

	if (traceEnabled)
	{
		pCamera->K2_SetRelativeLocation(HitTest(dt, restTfm), false, svcHitResult, true);
	}
	else
	{
		pCamera->K2_SetRelativeLocation(svcCamFake, false, svcHitResult, true);
	}
}

FVector UCameraSmartStickPhysical::HitTest(float dt, const FTransform& restTfm)
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

void UCameraSmartStickPhysical::SetRotLimits()
{
	stickIniRot = pCameraTarget->GetRelativeRotation();

	stickPitchMin = -stickPitchLimit - stickIniRot.Pitch;
	stickPitchMax = stickPitchLimit - stickIniRot.Pitch;
}

void UCameraSmartStickPhysical::RotateStick(float yaw, float pitch, float deltaTime)
{
	float alpha{ 1.0f / (1.0f + stickRotTau / deltaTime) };

	if (stickRotPermanent)
	{
		// --- Pitch ---
		stickPitchRate += alpha * (stickRotRate * pitch - stickPitchRate); // low-pass filtering
		stickPitch += deltaTime * LimitRate(stickPitchRate, stickPitch, stickPitchMin, stickPitchMax, stickRotRateFadeWidthR);

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

void UCameraSmartStickPhysical::RotateCamera(float yaw, float pitch, float deltaTime)
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

void UCameraSmartStickPhysical::ChangeDistance(float input, float deltaTime)
{
	float alpha{ 1.0f / (1.0f + telescopingTau / deltaTime) };
	telescopingRateCurrent += alpha * (telescopingRate * input - telescopingRateCurrent); // low-pass filtering

	float x{ pRestingPoint->GetRelativeLocation().X };
	x += deltaTime * LimitRate(telescopingRateCurrent, x, -distanceMax, -distanceMin, telescopingRateFadeWidth);

	pRestingPoint->K2_SetRelativeLocation(FVector(x, 0.0f, 0.0f), false, svcHitResult, true);
	inlUpdateStickRotRateFadeWidth(x);
}

void UCameraSmartStickPhysical::AxesPoolInput(FAxesPool input)
{
	if (!isTransferred)
	{
		ChangeDistance(input.Distance, input.DeltaTime);
		RotateStick(input.StickYaw, input.StickPitch, input.DeltaTime);
	}

	RotateCamera(input.CameraYaw, input.CameraPitch, input.DeltaTime);
}

void UCameraSmartStickPhysical::StorePosition(int32 index)
{
	presetPositions.Emplace(index, FDistanceRotator{ pRestingPoint->GetRelativeLocation().X , pCameraTarget->GetRelativeRotation() });
}

bool UCameraSmartStickPhysical::TransferToPreset(int32 index)
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

void UCameraSmartStickPhysical::TransferRestingPoint(float deltaTime)
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
			inlUpdateStickRotRateFadeWidth(pRestingPoint->GetRelativeLocation().X);

			stickPitch = 0.0f;
			stickYaw = 0.0f;
			stickPitchRate = 0.0f;
			stickYawRate = 0.0f;

			isTransferred = false;
		}
	}
}

void UCameraSmartStickPhysical::AdjustStickRotRateFadeWidth()
{
	inlAdjustStickRotRateFadeWidth();
}

void UCameraSmartStickPhysical::AdjustTelescopingFadeWidth()
{
	inlAdjustTelescopingFadeWidth();
}

// ---------------------------------------------------------------------------------------------------------------------
// --- BlueprintSetters ------------------------------------------------------------------------------------------------
void UCameraSmartStickPhysical::displacementMax_BPSet(float value) { SetDisplacementMax(value); }
void UCameraSmartStickPhysical::stiffness_BPSet(float value) { SetStiffness(value); }
void UCameraSmartStickPhysical::dampingRatio_BPSet(float value) { SetDampingRatio(value); }
void UCameraSmartStickPhysical::accelerationMax_BPSet(float value) { SetAccelerationMax(value); }
void UCameraSmartStickPhysical::displacementMargin_BPSet(float value) { SetDisplacementMargin(value); }
void UCameraSmartStickPhysical::stiffnessGain_BPSet(float value) { SetStiffnessGain(value); }

void UCameraSmartStickPhysical::traceRadius_BPSet(float value) { SetTraceRadius(value); }
void UCameraSmartStickPhysical::traceRate_BPSet(float value) { SetTraceRate(value); }
void UCameraSmartStickPhysical::traceTau_BPSet(float value) { SetTraceTau(value); }
void UCameraSmartStickPhysical::traceInterval_BPSet(float value) { SetTraceInterval(value); }

void UCameraSmartStickPhysical::stickRotPermanent_BPSet(bool value) { SetStickRotPermanent(value); }
void UCameraSmartStickPhysical::stickRotRate_BPSet(float value) { SetStickRotRate(value); }
void UCameraSmartStickPhysical::stickRotTau_BPSet(float value) { SetStickRotTau(value); }
void UCameraSmartStickPhysical::stickPitchLimit_BPSet(float value) { SetStickPitchLimit(value); }

void UCameraSmartStickPhysical::cameraRotRate_BPSet(float value) { SetCameraRotRate(value); }
void UCameraSmartStickPhysical::cameraRotTau_BPSet(float value) { SetCameraRotTau(value); }

void UCameraSmartStickPhysical::telescopingRate_BPSet(float value) { SetTelescopingRate(value); }
void UCameraSmartStickPhysical::telescopingTau_BPSet(float value) { SetTelescopingTau(value); }
void UCameraSmartStickPhysical::distanceMin_BPSet(float value) { SetDistanceMin(value); }
void UCameraSmartStickPhysical::distanceMax_BPSet(float value) { SetDistanceMax(value); }

