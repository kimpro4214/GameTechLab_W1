#include "pch.h"
#include "Physics/UBall.h"

#include <cmath>

UBall::UBall(
	const FVector& InitialLocation,
	const FVector& InitialVelocity,
	int InitialLevel,
	float InitialRadius)
	: Location(InitialLocation)
	, Velocity(InitialVelocity)
{
	SetLevel(InitialLevel, InitialRadius);
}

bool UBall::IsColliding(const UBall* OtherBall) const
{
	if (OtherBall == nullptr || OtherBall == this)
	{
		return false;
	}

	const FVector Delta = OtherBall->Location - Location;
	const float RadiusSum = Radius + OtherBall->Radius;
	return FVector::DotProduct(Delta, Delta) <= RadiusSum * RadiusSum;
}

void UBall::AddVelocity(const FVector& DeltaVelocity)
{
	Velocity += DeltaVelocity;
}

float UBall::GetMomentOfInertia() const
{
	return 0.5f * Mass * Radius * Radius;
}

void UBall::AddTorque(float Torque, float DeltaTime)
{
	const float AngularAcceleration = Torque / GetMomentOfInertia();
	AngularVelocity += AngularAcceleration * DeltaTime;
}

void UBall::Move(float DeltaTime, float AngularDamping)
{
	Location += Velocity * DeltaTime;
	AngularVelocity /= 1.0f + AngularDamping * DeltaTime;
	RotationAngle += AngularVelocity * DeltaTime;
	RotationAngle = fmodf(RotationAngle, 6.28318530718f);
}

void UBall::SetLevel(int NewLevel, float NewRadius)
{
	Level = NewLevel;
	SetRadius(NewRadius);
}

void UBall::SetRadius(float NewRadius)
{
	Radius = NewRadius > 0.0f ? NewRadius : 0.01f;
	Mass = 0.01f;
}
