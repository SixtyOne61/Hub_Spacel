// All rights reserved Zubov Alexander zubetto85@gmail.com 2019

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "XMath.generated.h"

/**
 * 
 */
UCLASS()
class HUB_SPACEL_API UXMath : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	/*
		Clamps the input in the range [-abs(amplitude), abs(amplitude)].
	*/
	static FORCEINLINE float inlClampf(float input, float amplitude)
	{
		if (abs(input) > abs(amplitude))
		{
			input = copysignf(amplitude, input);
		}

		return input;
	}

	/*
		Clamps the input in the range [-abs(amplitude), abs(amplitude)] and returns true if
		the input was clamped.
	*/
	static FORCEINLINE bool inlClampf(float input, float amplitude, float& output)
	{
		if (abs(input) > abs(amplitude))
		{
			output = copysignf(amplitude, input);
			return true;
		}
		else
		{
			output = input;
			return false;
		}
	}

	/*
		Clamps the input in the range [-abs(amplitude), abs(amplitude)] and returns true if
		the input was clamped.
	*/
	UFUNCTION(BlueprintPure, Category = "XMath")
	static bool Clampf(float input, float amplitude, float& output);


	/*
		Clamps each component of the given vector into the range set by corresponding component of the amps vector,
		e.g. x-component of the vector will be clamped into the range [-abs(amps.x), abs(amps.x)].
	*/
	static FORCEINLINE void inlClamp(FVector& vector, const FVector& amps)
	{
		if (abs(vector.X) > abs(amps.X))
			vector.X = copysignf(amps.X, vector.X);

		if (abs(vector.Y) > abs(amps.Y))
			vector.Y = copysignf(amps.Y, vector.Y);

		if (abs(vector.Z) > abs(amps.Z))
			vector.Z = copysignf(amps.Z, vector.Z);
	}

	/*
		Clamps each component of the given vector into the range set by corresponding component of the amps vector,
		e.g. x-component of the vector will be clamped into the range [-abs(amps.x), abs(amps.x)].
	*/
	UFUNCTION(BlueprintCallable, Category = "XMath")
	static FVector& ClampRef(UPARAM(ref) FVector& vector, const FVector& amps);

	/*
		Returns clamped copy of the given vector,
		e.g. x-component of the vector will be clamped into the range [-abs(amps.x), abs(amps.x)].
	*/
	UFUNCTION(BlueprintPure, Category = "XMath")
	static FVector Clamp(const FVector& vector, const FVector& amps);


	static FORCEINLINE FVector inlToRadians(const FRotator& rot)
	{
		constexpr float DtR = PI / 180.0f;

		return FVector(DtR * rot.Roll, DtR * rot.Pitch, DtR * rot.Yaw);
	}

	UFUNCTION(BlueprintPure, Category = "XMath")
	static FVector ToRadians(const FRotator& rot);


	/*
		For the given angles returns Bn such that abs(Bn - A) <= abs(B - A).
		Expects angles in degrees within the range [-180, 180]
	*/
	static FORCEINLINE float inlNearestAngle(float A, float B)
	{
		float delta{ B - A };
		float deltaRev{ delta + copysignf(360.0f, A) };

		if (fabsf(deltaRev) < fabsf(delta))
			return deltaRev + A;
		else
			return B;
	}

	/*
		For the given angles returns Bn such that abs(Bn - A) <= abs(B - A).
		Expects angles in degrees within the range [-180, 180]
	*/
	UFUNCTION(BlueprintPure, Category = "XMath")
	static float NearestAngle(float A, float B);


	static FORCEINLINE FVector inlCopySign(const FVector& target, const FVector& source)
	{
		return FVector(copysignf(target.X, source.X), copysignf(target.Y, source.Y), copysignf(target.Z, source.Z));
	}

	UFUNCTION(BlueprintPure, Category = "XMath")
	static FVector CopySign(const FVector& target, const FVector& source);


	UFUNCTION(BlueprintPure, Category = "XMath")
	static float CopySignf(float target, float source);


	static void FORCEINLINE inlSetSign(FVector& target, const FVector& source)
	{
		target.X = copysignf(target.X, source.X);
		target.Y = copysignf(target.Y, source.Y);
		target.Z = copysignf(target.Z, source.Z);
	}

	UFUNCTION(BlueprintCallable, Category = "XMath")
	static void SetSign(UPARAM(ref) FVector& target, const FVector& source);


	/*
		Computes timing for triangular motion profile with the desired acceleration.

		Parameters:
		deltaX - x-coordinate difference between the target location and current location;
		Vx - x-coordinate of velocity at current location (deltaX decreases if it has the same sign as Vx);
		Aabs - absolute value of desired acceleration;

		Returns:
		Ax - unmodified absolute value of the desired acceleration with the same sign as deltaX;
		timing.X - time remaining before switching from acceleration to deceleration;
		timing.Y - time remaining before stopping at the target location;

	*/
	UFUNCTION(BlueprintCallable, Category = "XMath")
	static UPARAM(DisplayName = "Ax") float ComputeTimingForTriangularMP(float deltaX, float Vx, float Aabs, FVector2D& timing);


	/*
		Computes timing for triangular motion profile with adaptive acceleration, that is, 
		acceleration is close to zero when Vx and deltaX are close to zero:
		https://www.desmos.com/calculator/lrkce3tsrg

		Parameters:
		deltaX - x-coordinate difference between the target location and current location;
		Vx - x-coordinate of velocity at current location (deltaX decreases if it has the same sign as Vx);
		Tswitch - desired time interval until switching from acceleration to deceleration;

		Returns:
		Ax - adapted acceleration with the same sign as deltaX;
		timing.X - time remaining before switching from acceleration to deceleration;
		timing.Y - time remaining before stopping at the target location;
	*/
	UFUNCTION(BlueprintCallable, Category = "XMath")
	static UPARAM(DisplayName = "Ax") float ComputeTimingForTriangularMPAA(float deltaX, float Vx, float Tswitch, FVector2D& timing);


	/*
		Three angles overload of the method UXMath::ConjugateTwoParabolas(float deltaX, float Vx, float Ax);
		The Tswitch and Tstop vectors will contain switching and stopping times for each angle, omega and epsilon from corresponding vectors;

		angles - Roll, Pitch, Yaw offsets in radians from the desired attitude;
		omegas - approach angular velocities in radians per second, if its element has the same sign as the corresponding angle offset,
				 then causes a decrease of this angle offset;
		epsilons - desired angular accelerations of boost and retard in radians per second squared,
				   after this method call it matches the boost;
	*/
	UFUNCTION(BlueprintCallable, Category = "XMath")
	static void ComputeTimingForTriangularRP(const FVector& angles, const FVector& omegas, UPARAM(ref) FVector& epsilons, FVector& Tswitch, FVector& Tstop);


	/*

	*/
	UFUNCTION(BlueprintPure, Category = "XMath")
	static float ForceOnSchedule(float Tswitch, float Tstop, float tick, float boost);


	/**/
	UFUNCTION(BlueprintCallable, Category = "XMath")
	static UPARAM(DisplayName = "Scheduled Acceleration") FVector& TorqueOnSchedule(const FVector& Tswitch, const FVector& Tstop, float tick, UPARAM(ref) FVector& boost);


	/**/
	UFUNCTION(BlueprintPure, Category = "XMath")
	static UPARAM(DisplayName = "Scheduled Acceleration") float BoostRetardController(float deltaX, float Vx, float Amax, float Tswitch, float tick);


	/**/
	UFUNCTION(BlueprintPure, Category = "XMath")
	static UPARAM(DisplayName = "Scheduled Acceleration") FVector BoostRetardControllerRot(const FVector& angles, const FVector& omegas, const FVector& MaxEpsilons, float Tswitch, float tick);


	static FORCEINLINE float inlLowPassFilterAlpha(float timeConstant, float deltaTime)
	{
		return 1.0f / (1.0f + timeConstant / deltaTime);
	}


	/**/
	UFUNCTION(BlueprintPure, Category = "XMath")
	static UPARAM(DisplayName = "Alpha") float LowPassFilterAlpha(float timeConstant, float deltaTime);


	template<typename T>
	static FORCEINLINE void inlLowPassFilter(const T& currentX, T& previousY, float alpha = 0.5f)
	{
		previousY += alpha * (currentX - previousY);
	}


	/*
		Discrete-time implementation of a simple RC low-pass filter.
		Returns currentY = previousY + alpha * (currentX - previuosY)

		currentX - current input value;
		previousY - previous output value, after call will be assigned to currentY;
		alpha - smoothing factor is chosen from the range [0, 1]
		close to 0 - strong smoothing
		close to 1 - slight smoothing
	*/
	UFUNCTION(BlueprintCallable, Category = "XMath")
	static void LowPassFilterFloat(float currentX, UPARAM(ref) float& previousY, float& currentY, float alpha = 0.5f);


	/*
		Discrete-time implementation of a simple RC low-pass filter.
		Returns currentY = previousY + alpha * (currentX - previuosY)

		currentX - current input value;
		previousY - previous output value, after call will be assigned to currentY;
		alpha - smoothing factor is chosen from the range [0, 1]
		close to 0 - strong smoothing
		close to 1 - slight smoothing
	*/
	UFUNCTION(BlueprintCallable, Category = "XMath")
	static void LowPassFilterVec(const FVector& currentX, UPARAM(ref) FVector& previousY, FVector& currentY, float alpha = 0.5f);

	/*
		Wrapper method for the FQuat::Slerp(FQuat current, FQuat target, float) which
		avoids the problem of gimbal lock
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "XMath")
	static FRotator RInterpTo(const FRotator& current, const FRotator& target, float deltaTime, float interpSpeed);

	/*
		Interpolates between rotatations of the current and the target transforms
	*/
	UFUNCTION(BlueprintCallable, Category = "XMath")
	static UPARAM(DisplayName = "Current Out") FTransform& TrotInterpTo(UPARAM(ref) FTransform& current, const FTransform& target, float deltaTime, float interpSpeed);

	/*
		Low-pass filter of a USceneComponent rotations.
		Does not take into account and does not affect physics.

		current - USceneComponent with smoothed rotations of the target
		alpha - smoothing factor is chosen from the range [0, 1]
		close to 0 - strong smoothing
		close to 1 - slight smoothing
	*/
	UFUNCTION(BlueprintCallable, Category = "XMath", meta = (KeyWords = "Low-pass filter component rotation"))
	static void ComponentRotLPF(UPARAM(ref) USceneComponent* current, const USceneComponent* target, float alpha);

	/*
		Maps given angle, expected in the range [-360, 360], to the range (-180, 180]. Angles are in degrees
	*/
	static FORCEINLINE float inlNormalizeClampedAngleD(float angle)
	{
		if (fabsf(angle) > 180.0f)
			angle -= copysignf(360.0f, angle);

		return angle;
	}

	/*
		Maps given angle, expected in the range [-360, 360], to the range (-180, 180]. Angles are in degrees
	*/
	UFUNCTION(BlueprintPure, Category = "XMath")
	static float NormalizeClampedAngleD(float angle);


	/**/
	static FORCEINLINE FVector& inlStepWithConstAcceleration(const FVector& A, FVector& Vo, FVector& Xo, float deltaTime)
	{
		Xo += Vo * deltaTime + A * deltaTime * deltaTime / 2.0f;
		Vo += A * deltaTime;
		return Xo;
	}


	/**/
	UFUNCTION(BlueprintCallable, Category = "XMath")
	static UPARAM(DisplayName = "X") FVector& StepWithConstAcceleration(const FVector& A, UPARAM(ref) FVector& Vo, UPARAM(ref) FVector& Xo, float deltaTime);
};
