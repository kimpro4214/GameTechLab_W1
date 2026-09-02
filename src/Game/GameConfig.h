#pragma once

#include "FVector.h"

#include <chrono>

namespace GameConfig
{
	inline constexpr float LeftBorder = -1.0f;
	inline constexpr float RightBorder = 0.5f;
	inline constexpr float TopBorder = -1.0f;
	inline constexpr float BottomBorder = 1.0f;
	inline constexpr float GameOverLineY = 0.8f;

	inline const FVector GravityAcceleration(0.0f, -9.81f, 0.0f);
	inline constexpr float Restitution = 0.2f;
	inline constexpr float FrictionCoefficient = 0.5f;
	inline constexpr float AngularDamping = 0.1f;
	inline constexpr int PhysicsSubsteps = 2;
	inline constexpr int CollisionSolverIterations = 8;
	inline constexpr std::chrono::milliseconds DropCooldown(600);
	inline constexpr std::chrono::milliseconds StoreCooldown(300);
	inline constexpr float DropHorizontalJitter = 0.005f;

	inline constexpr int TargetFps = 30;
}
