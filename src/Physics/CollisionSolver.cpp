#include "Physics/CollisionSolver.h"

#include "Physics/UBall.h"

#include <cmath>

void CollisionSolver::ResolveBallCollision(
	UBall& BallA,
	UBall& BallB,
	float Restitution,
	float FrictionCoefficient,
	bool  bHasMerged)
{
	const FVector Delta = BallB.Location - BallA.Location;
	const float RadiusSum = BallA.Radius + BallB.Radius;
	const float DistanceSquared = FVector::DotProduct(Delta, Delta);
	if (DistanceSquared > RadiusSum * RadiusSum)
	{
		return;
	}

	const FVector RelativeVelocity = BallB.Velocity - BallA.Velocity;
	float Distance = 0.0f;
	FVector CollisionNormal(1.0f, 0.0f, 0.0f);
	constexpr float Epsilon = 0.000001f;

	if (DistanceSquared > Epsilon)
	{
		Distance = sqrtf(DistanceSquared);
		CollisionNormal = Delta / Distance;
	}
	else
	{
		const float RelativeSpeedSquared = FVector::DotProduct(RelativeVelocity, RelativeVelocity);
		if (RelativeSpeedSquared > Epsilon)
		{
			CollisionNormal = RelativeVelocity * (-1.0f / sqrtf(RelativeSpeedSquared));
		}
	}

	const float InverseMassA = 1.0f / BallA.Mass;
	const float InverseMassB = 1.0f / BallB.Mass;
	const float InverseInertiaA = 1.0f / BallA.GetMomentOfInertia();
	const float InverseInertiaB = 1.0f / BallB.GetMomentOfInertia();
	const float InverseMassSum = InverseMassA + InverseMassB;

	const float Penetration = RadiusSum - Distance;
	constexpr float PenetrationSlop = 0.001f;
	constexpr float CorrectionPercent = 0.25f;
	const float CorrectedPenetration =
		Penetration > PenetrationSlop ? Penetration - PenetrationSlop : 0.0f;
	const FVector Correction =
		CollisionNormal * (CorrectedPenetration * CorrectionPercent / InverseMassSum);
	BallA.Location -= Correction * InverseMassA;
	BallB.Location += Correction * InverseMassB;

	if (bHasMerged)
	{
		const float PenetrationVelocityScale = 500.0f;
		const float MaxPenetrationVelocity = 10.0f;

		float PenetrationImpulseMagnitude =
			CorrectedPenetration * PenetrationVelocityScale > MaxPenetrationVelocity ? MaxPenetrationVelocity : CorrectedPenetration * PenetrationVelocityScale;
		const FVector PenetrationImpulse = CollisionNormal * (PenetrationImpulseMagnitude / InverseMassSum);
		BallA.Velocity -= PenetrationImpulse * InverseMassA;
		BallB.Velocity += PenetrationImpulse * InverseMassB;
	}

	const FVector ContactOffsetA = CollisionNormal * BallA.Radius;
	const FVector ContactOffsetB = CollisionNormal * -BallB.Radius;
	const FVector ContactVelocityA =
		BallA.Velocity + FVector::CrossProduct2D(BallA.AngularVelocity, ContactOffsetA);
	const FVector ContactVelocityB =
		BallB.Velocity + FVector::CrossProduct2D(BallB.AngularVelocity, ContactOffsetB);
	const FVector RelativeContactVelocity = ContactVelocityB - ContactVelocityA;
	const float VelocityAlongNormal =
		FVector::DotProduct(RelativeContactVelocity, CollisionNormal);

	BallA.LastCollisionPoint = BallA.Location + ContactOffsetA;
	BallA.LastCollisionNormal = CollisionNormal;
	BallA.LastCollisionTangent = FVector(-CollisionNormal.y, CollisionNormal.x, 0.0f);
	BallA.bHasCollisionDebug = true;
	BallB.LastCollisionPoint = BallA.LastCollisionPoint;
	BallB.LastCollisionNormal = CollisionNormal * -1.0f;
	BallB.LastCollisionTangent = BallA.LastCollisionTangent;
	BallB.bHasCollisionDebug = true;

	if (VelocityAlongNormal >= 0.0f)
	{
		return;
	}

	const float NormalCrossA = FVector::CrossProduct2D(ContactOffsetA, CollisionNormal);
	const float NormalCrossB = FVector::CrossProduct2D(ContactOffsetB, CollisionNormal);
	const float NormalDenominator =
		InverseMassSum +
		NormalCrossA * NormalCrossA * InverseInertiaA +
		NormalCrossB * NormalCrossB * InverseInertiaB;
	if (NormalDenominator <= Epsilon)
	{
		return;
	}

	const float NormalImpulseMagnitude =
		-(1.0f + Restitution) * VelocityAlongNormal / NormalDenominator;
	const FVector NormalImpulse = CollisionNormal * NormalImpulseMagnitude;
	BallA.Velocity -= NormalImpulse * InverseMassA;
	BallB.Velocity += NormalImpulse * InverseMassB;
	BallA.AngularVelocity -=
		FVector::CrossProduct2D(ContactOffsetA, NormalImpulse) * InverseInertiaA;
	BallB.AngularVelocity +=
		FVector::CrossProduct2D(ContactOffsetB, NormalImpulse) * InverseInertiaB;

	const FVector NewContactVelocityA =
		BallA.Velocity + FVector::CrossProduct2D(BallA.AngularVelocity, ContactOffsetA);
	const FVector NewContactVelocityB =
		BallB.Velocity + FVector::CrossProduct2D(BallB.AngularVelocity, ContactOffsetB);
	const FVector NewRelativeVelocity = NewContactVelocityB - NewContactVelocityA;
	const float NewVelocityAlongNormal =
		FVector::DotProduct(NewRelativeVelocity, CollisionNormal);
	const FVector TangentVelocity =
		NewRelativeVelocity - CollisionNormal * NewVelocityAlongNormal;
	const float TangentSpeedSquared = FVector::DotProduct(TangentVelocity, TangentVelocity);
	if (TangentSpeedSquared <= Epsilon)
	{
		return;
	}

	const FVector Tangent = TangentVelocity / sqrtf(TangentSpeedSquared);
	BallA.LastCollisionTangent = Tangent;
	BallB.LastCollisionTangent = Tangent;
	const float TangentCrossA = FVector::CrossProduct2D(ContactOffsetA, Tangent);
	const float TangentCrossB = FVector::CrossProduct2D(ContactOffsetB, Tangent);
	const float TangentDenominator =
		InverseMassSum +
		TangentCrossA * TangentCrossA * InverseInertiaA +
		TangentCrossB * TangentCrossB * InverseInertiaB;
	if (TangentDenominator <= Epsilon)
	{
		return;
	}

	float TangentImpulseMagnitude =
		-FVector::DotProduct(NewRelativeVelocity, Tangent) / TangentDenominator;
	const float MaxFrictionImpulse = FrictionCoefficient * NormalImpulseMagnitude;
	if (TangentImpulseMagnitude > MaxFrictionImpulse)
	{
		TangentImpulseMagnitude = MaxFrictionImpulse;
	}
	else if (TangentImpulseMagnitude < -MaxFrictionImpulse)
	{
		TangentImpulseMagnitude = -MaxFrictionImpulse;
	}

	const FVector FrictionImpulse = Tangent * TangentImpulseMagnitude;
	BallA.Velocity -= FrictionImpulse * InverseMassA;
	BallB.Velocity += FrictionImpulse * InverseMassB;
	BallA.AngularVelocity -=
		FVector::CrossProduct2D(ContactOffsetA, FrictionImpulse) * InverseInertiaA;
	BallB.AngularVelocity +=
		FVector::CrossProduct2D(ContactOffsetB, FrictionImpulse) * InverseInertiaB;
}

void CollisionSolver::ResolveBorderContact(
	UBall& Ball,
	const FVector& CollisionNormal,
	float Restitution,
	float FrictionCoefficient)
{
	Ball.bHasCollisionThisFrame = true;
	constexpr float Epsilon = 0.000001f;
	const float InverseMass = 1.0f / Ball.Mass;
	const float InverseInertia = 1.0f / Ball.GetMomentOfInertia();
	const FVector ContactOffset = CollisionNormal * -Ball.Radius;
	FVector ContactVelocity =
		Ball.Velocity + FVector::CrossProduct2D(Ball.AngularVelocity, ContactOffset);
	const float VelocityAlongNormal = FVector::DotProduct(ContactVelocity, CollisionNormal);
	if (VelocityAlongNormal >= 0.0f)
	{
		return;
	}

	const float NormalCross = FVector::CrossProduct2D(ContactOffset, CollisionNormal);
	const float NormalDenominator =
		InverseMass + NormalCross * NormalCross * InverseInertia;
	if (NormalDenominator <= Epsilon)
	{
		return;
	}

	const float NormalImpulseMagnitude =
		-(1.0f + Restitution) * VelocityAlongNormal / NormalDenominator;
	const FVector NormalImpulse = CollisionNormal * NormalImpulseMagnitude;
	Ball.Velocity += NormalImpulse * InverseMass;
	Ball.AngularVelocity +=
		FVector::CrossProduct2D(ContactOffset, NormalImpulse) * InverseInertia;

	ContactVelocity =
		Ball.Velocity + FVector::CrossProduct2D(Ball.AngularVelocity, ContactOffset);
	const float NewVelocityAlongNormal = FVector::DotProduct(ContactVelocity, CollisionNormal);
	const FVector TangentVelocity =
		ContactVelocity - CollisionNormal * NewVelocityAlongNormal;
	const float TangentSpeedSquared = FVector::DotProduct(TangentVelocity, TangentVelocity);
	if (TangentSpeedSquared <= Epsilon)
	{
		return;
	}

	const FVector Tangent = TangentVelocity / sqrtf(TangentSpeedSquared);
	const float TangentCross = FVector::CrossProduct2D(ContactOffset, Tangent);
	const float TangentDenominator =
		InverseMass + TangentCross * TangentCross * InverseInertia;
	if (TangentDenominator <= Epsilon)
	{
		return;
	}

	float TangentImpulseMagnitude =
		-FVector::DotProduct(ContactVelocity, Tangent) / TangentDenominator;
	const float MaxFrictionImpulse = FrictionCoefficient * NormalImpulseMagnitude;
	if (TangentImpulseMagnitude > MaxFrictionImpulse)
	{
		TangentImpulseMagnitude = MaxFrictionImpulse;
	}
	else if (TangentImpulseMagnitude < -MaxFrictionImpulse)
	{
		TangentImpulseMagnitude = -MaxFrictionImpulse;
	}

	const FVector FrictionImpulse = Tangent * TangentImpulseMagnitude;
	Ball.Velocity += FrictionImpulse * InverseMass;
	Ball.AngularVelocity +=
		FVector::CrossProduct2D(ContactOffset, FrictionImpulse) * InverseInertia;
}
