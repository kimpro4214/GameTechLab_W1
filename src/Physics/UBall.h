#pragma once

#include "FVector.h"

class UBall
{
public:
	UBall(
		const FVector& InitialLocation,
		const FVector& InitialVelocity,
		int InitialLevel,
		float InitialRadius);

	UBall(const UBall&) = delete;
	UBall& operator=(const UBall&) = delete;

	bool IsColliding(const UBall* Other) const;
	void AddVelocity(const FVector& DeltaVelocity);

	float GetMomentOfInertia() const;
	void AddTorque(float Torque, float DeltaTime);
	void Move(float DeltaTime, float AngularDamping);
	void SetLevel(int NewLevel, float NewRadius);

	FVector Location;
	FVector Velocity;
	float Radius = 0.0f;
	float Mass = 0.0f;
	float RotationAngle = 0.0f;
	float AngularVelocity = 0.0f;
	FVector LastCollisionPoint;
	FVector LastCollisionNormal;
	FVector LastCollisionTangent;
	bool bHasCollisionDebug = false;
	bool bHasCollisionThisFrame = false;
	bool bHasTouchedSomething = false;
	bool bHasBeenDropped = false;
	bool bIsMerging = false;
	int Level = 0;

private:
	void SetRadius(float NewRadius);
};
