#pragma once

class UBall;
struct FVector;

class CollisionSolver
{
public:
	static void ResolveBallCollision(
		UBall& BallA,
		UBall& BallB,
		float Restitution,
		float FrictionCoefficient);
	static void ResolveBorderContact(
		UBall& Ball,
		const FVector& CollisionNormal,
		float Restitution,
		float FrictionCoefficient);
};
