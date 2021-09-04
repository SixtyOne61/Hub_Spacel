// All rights reserved Zubov Alexander zubetto85@gmail.com 2019


#include "XMath.h"

bool UXMath::Clampf(float input, float amplitude, float& output)
{
	return inlClampf(input, amplitude, output);
}

FVector& UXMath::ClampRef(FVector& vector, const FVector& amps)
{
	inlClamp(vector, amps);

	return vector;
}

FVector UXMath::Clamp(const FVector& vector, const FVector& amps)
{
	FVector copyV{ vector };
	inlClamp(copyV, amps);

	return copyV;
}

FVector UXMath::ToRadians(const FRotator& rot)
{
	return inlToRadians(rot);
}

float UXMath::NearestAngle(float A, float B)
{
	return inlNearestAngle(A, B);
}

FVector UXMath::CopySign(const FVector& target, const FVector& source)
{
	return inlCopySign(target, source);
}

float UXMath::CopySignf(float target, float source)
{
	return copysignf(target, source);
}

void UXMath::SetSign(FVector& target, const FVector& source)
{
	inlSetSign(target, source);
}

float UXMath::ComputeTimingForTriangularMP(float deltaX, float Vx, float Aabs, FVector2D& timing)
{
	float Ax{ copysignf(Aabs, deltaX) };

	float Tcomp{ Vx / Ax };

	float Tswitch{ sqrtf(0.5f * Tcomp * Tcomp + deltaX / Ax) - Tcomp };

	// Set Tswitch and Tstop
	timing.X = Tswitch;
	timing.Y = 2.0f * Tswitch + Tcomp;

	return Ax;
}

float UXMath::ComputeTimingForTriangularMPAA(float deltaX, float Vx, float Tswitch, FVector2D& timing)
{
	Tswitch = abs(Tswitch);
	float Ax{ 0.0f }; // adaptive acceleration

	// Actual time remaining before switching written as function of deltaX, Vx and Ax:
	// Fsw(deltaX, Vx, Ax) = sqrt(0.5*(Vx/Ax)^2 + deltaX/Ax) - Vx/Ax

	if (signbit(deltaX) == signbit(Vx))
	{
		// then adaptive acceleration is set linearly proportional to deltaX 
		// with the coefficient of proportionality k chosen so as to satisfy
		// both of the following equation and inequality:
		// Fsw(deltaX, 0, k*deltaX) = Tswitch,
		// Fsw(deltaX, Vx, k*deltaX) >= 0;

		// Solving the first equation for k, we get k = 1 / Tswitch^2
		float k{ 1.0f / (Tswitch * Tswitch) };

		// From the second inequality we get the lower limit for k: 0.5*(Vx/deltaX)^2
		float Klower{ Vx / deltaX };
		Klower *= Klower/2.0f;

		k = fmaxf(Klower, k);
		Ax = k * deltaX;
	}
	else
	{
		// adaptive acceleration Ax is chosen so as to satisfy the equation
		// Fsw(deltaX, Vx, Ax) = Tswitch, which can be rewritten as follows:
		// Ax^2 + (2*Vx*Tswitch - deltaX)/Tswitch^2 * Ax + 0.5*(Vx/Tswitch)^2 = 0;
		// using the Grant Sanderson method (m + - sqrt (m * m - p)), we find Ax:
		float m{ (deltaX - 2.0f * Vx * Tswitch) / (2.0f * Tswitch * Tswitch) };
		float p{ Vx / Tswitch };
		p *= p / 2.0f;

		Ax = m + copysignf(sqrt(m * m - p), deltaX);
	}

	float Tcomp{ Vx / Ax };

	// Actual Tswitch for the given deltaX, Vx and Ax:
	Tswitch = sqrtf(0.5f * Tcomp * Tcomp + deltaX / Ax) - Tcomp;

	// Set Tswitch and Tstop
	timing.X = Tswitch;
	timing.Y = 2.0f * Tswitch + Tcomp;

	return Ax;
}

void UXMath::ComputeTimingForTriangularRP(const FVector& angles, const FVector& omegas, FVector& epsilons, FVector& Tswitch, FVector& Tstop)
{
	SetSign(epsilons, angles);

	// --- ROLL --------------------------------------
	float Tcomp{ omegas.X / epsilons.X };

	Tswitch.X = sqrtf(0.5f * Tcomp * Tcomp + angles.X / epsilons.X) - Tcomp;
	Tstop.X = 2.0f * Tswitch.X + Tcomp;

	// --- PITCH -------------------------------------
	Tcomp = omegas.Y / epsilons.Y;

	Tswitch.Y = sqrtf(0.5f * Tcomp * Tcomp + angles.Y / epsilons.Y) - Tcomp;
	Tstop.Y = 2.0f * Tswitch.Y + Tcomp;

	// --- YAW ---------------------------------------
	Tcomp = omegas.Z / epsilons.Z;

	Tswitch.Z = sqrtf(0.5f * Tcomp * Tcomp + angles.Z / epsilons.Z) - Tcomp;
	Tstop.Z = 2.0f * Tswitch.Z + Tcomp;
}

float UXMath::ForceOnSchedule(float Tswitch, float Tstop, float tick, float boost)
{
	if (Tstop >= tick)
	{
		if (Tswitch > tick)
			return boost;
		else
			return -boost;
	}
	else
		return 0.0f;
}

FVector& UXMath::TorqueOnSchedule(const FVector& Tswitch, const FVector& Tstop, float tick, FVector& boost)
{
	// --- ROLL -----------------------------------------
	if (Tstop.X >= tick)
		boost.X = Tswitch.X > tick ? -boost.X : boost.X;
	else
		boost.X = 0.0f;

	// --- PITCH ----------------------------------------
	if (Tstop.Y >= tick)
		boost.Y = Tswitch.Y > tick ? -boost.Y : boost.Y;
	else
		boost.Y = 0.0f;

	// --- YAW ------------------------------------------
	if (Tstop.Z >= tick)
		boost.Z = Tswitch.Z > tick ? boost.Z : -boost.Z;
	else
		boost.Z = 0.0f;

	return boost;
}

float UXMath::BoostRetardController(float deltaX, float Vx, float Amax, float Tswitch, float tick)
{
	FVector2D timing;

	float accel{ ComputeTimingForTriangularMPAA(deltaX, Vx, Tswitch, timing) };
	accel = inlClampf(accel, Amax);

	return ForceOnSchedule(timing.X, timing.Y, tick, accel);
}

FVector UXMath::BoostRetardControllerRot(const FVector& angles, const FVector& omegas, const FVector& maxEpsilons, float Tswitch, float tick)
{
	FVector2D timing;
	FVector actualTswitch;
	FVector actualTstop;
	FVector epsilons;

	epsilons.X = ComputeTimingForTriangularMPAA(angles.X, omegas.X, Tswitch, timing);
	actualTswitch.X = timing.X;
	actualTstop.X = timing.Y;

	epsilons.Y = ComputeTimingForTriangularMPAA(angles.Y, omegas.Y, Tswitch, timing);
	actualTswitch.Y = timing.X;
	actualTstop.Y = timing.Y;

	epsilons.Z = ComputeTimingForTriangularMPAA(angles.Z, omegas.Z, Tswitch, timing);
	actualTswitch.Z = timing.X;
	actualTstop.Z = timing.Y;

	inlClamp(epsilons, maxEpsilons);

	return TorqueOnSchedule(actualTswitch, actualTstop, tick, epsilons);
}

float UXMath::LowPassFilterAlpha(float timeConstant, float deltaTime)
{
	return UXMath::inlLowPassFilterAlpha(timeConstant, deltaTime);
}

void UXMath::LowPassFilterFloat(float currentX, float& previousY, float& currentY, float alpha)
{
	UXMath::inlLowPassFilter<float>(currentX, previousY, alpha);
	currentY = previousY;
}

void UXMath::LowPassFilterVec(const FVector& currentX, FVector& previousY, FVector& currentY, float alpha)
{
	UXMath::inlLowPassFilter<FVector>(currentX, previousY, alpha);
	currentY = previousY;
}

FRotator UXMath::RInterpTo(const FRotator& current, const FRotator& target, float deltaTime, float interpSpeed)
{
	float alpha{ deltaTime * interpSpeed };

	if (alpha <= 0.0)
	{
		return current;
	}
	else if (alpha >= 1.0)
	{
		return target;
	}
	else
	{
		return FRotator(FQuat::Slerp(FQuat(current), FQuat(target), alpha));
	}
}

FTransform& UXMath::TrotInterpTo(FTransform& current, const FTransform& target, float deltaTime, float interpSpeed)
{
	float alpha{ deltaTime * interpSpeed };

	if (alpha <= 0.0)
	{
		return current;
	}
	else if (alpha >= 1.0)
	{
		current = target;
		return current;
	}
	else
	{
		current.SetRotation(FQuat::Slerp(current.GetRotation(), target.GetRotation(), alpha));
		return current;
	}
}

void UXMath::ComponentRotLPF(USceneComponent* current, const USceneComponent* target, float alpha)
{
	if (alpha <= 0.0)
	{
		return;
	}
	else if (alpha >= 1.0)
	{
		current->SetWorldRotation(target->GetComponentQuat(), false, nullptr, ETeleportType::TeleportPhysics);
	}
	else
	{
		FQuat newQuat{ FQuat::Slerp(current->GetComponentQuat(), target->GetComponentQuat(), alpha) };
		current->SetWorldRotation(newQuat, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

float UXMath::NormalizeClampedAngleD(float angle)
{
	return inlNormalizeClampedAngleD(angle);
}

FVector& UXMath::StepWithConstAcceleration(const FVector& A, FVector& Vo, FVector& Xo, float deltaTime)
{
	return UXMath::inlStepWithConstAcceleration(A, Vo, Xo, deltaTime);
}