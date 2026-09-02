#pragma once

#include "FVector.h"

#include <chrono>

namespace GameConfig
{
	inline constexpr float LeftBorder = -1.0f;
	inline constexpr float RightBorder = 0.5f;
	// The taller 1024x1280 window shows a 2.5-unit-high play field.
	inline constexpr float TopBorder = -1.25f;
	inline constexpr float BottomBorder = 1.25f;
	inline constexpr float GameOverLineY = 1.0f;
	inline constexpr float BallSpawnY = 1.125f;

	inline const FVector GravityAcceleration(0.0f, -9.81f, 0.0f);
	inline constexpr float Restitution = 0.2f;
	inline constexpr float FrictionCoefficient = 0.5f;
	inline constexpr float AngularDamping = 0.1f;
	inline constexpr int PhysicsSubsteps = 2;
	inline constexpr int CollisionSolverIterations = 8;
	inline constexpr std::chrono::milliseconds DropCooldown(600);
	inline constexpr float DropHorizontalJitter = 0.005f;

	inline constexpr float WaterSurfaceY = 0.5f;
	inline constexpr float UnderwaterGravityScale = 0.25f;
	inline constexpr float UnderwaterVelocityDamping = 0.90f;

	inline constexpr int TargetFps = 30;
}
