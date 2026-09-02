#pragma once

#include "FVector.h"

#include <functional>
#include <memory>
#include <vector>

class UBall;

struct FPhysicsWorldSettings
{
	FVector GravityAcceleration;
	float LeftBorder = -1.0f;
	float RightBorder = 1.0f;
	float TopBorder = -1.0f;
	float BottomBorder = 1.0f;
	float Restitution = 0.2f;
	float FrictionCoefficient = 0.5f;
	float AngularDamping = 0.1f;
	int Substeps = 1;
	int SolverIterations = 1;
};

class PhysicsWorld
{
public:
	using FBallMergeHandler = std::function<bool(UBall&, UBall&)>;
	explicit PhysicsWorld(const FPhysicsWorldSettings& InitialSettings);

	void Step(
		std::vector<std::unique_ptr<UBall>>& Balls,
		float DeltaTime,
		const FBallMergeHandler& TryMerge) const;

private:
	void ResolveBallCollisions(
		std::vector<std::unique_ptr<UBall>>& Balls,
		const FBallMergeHandler& TryMerge) const;
	void ResolveBorderCollisions(std::vector<std::unique_ptr<UBall>>& Balls) const;

	FPhysicsWorldSettings Settings;
};
