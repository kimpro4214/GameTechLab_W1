#include "pch.h"
#include "Physics/PhysicsWorld.h"

#include "Physics/CollisionSolver.h"
#include "Physics/UBall.h"

#include "Game/FruitCatalog.h"
#include "Game/GameConfig.h"

#include <algorithm>

PhysicsWorld::PhysicsWorld(const FPhysicsWorldSettings& InitialSettings)
	: Settings(InitialSettings)
{
}

void PhysicsWorld::Step(
	std::vector<std::unique_ptr<UBall>>& Balls,
	float DeltaTime,
	const FBallMergeHandler& TryMerge) const
{
	for (const std::unique_ptr<UBall>& Ball : Balls)
	{
		Ball->bHasCollisionDebug = false;
		Ball->bHasCollisionThisFrame = false;
	}

	const float SubstepDeltaTime = DeltaTime / static_cast<float>(Settings.Substeps);
	const FVector GravityVelocityChange = Settings.GravityAcceleration * SubstepDeltaTime;
	for (int Substep = 0; Substep < Settings.Substeps; ++Substep)
	{
		for (const std::unique_ptr<UBall>& Ball : Balls)
		{
			if (!Ball->bHasBeenDropped || Ball->bIsMerging)
			{
				continue;
			}

			Ball->AddVelocity(GravityVelocityChange);

			const float BallBottomY = Ball->Location.y - Ball->Radius;

			const float SubmergedRatio = std::clamp(
				(GameConfig::WaterSurfaceY - BallBottomY) / (2.0f * Ball->Radius),
				0.0f,
				1.0f);

			
			const float SizeRatio = std::clamp(
				(Ball->Radius * Ball->Radius) /
					(GameConfig::LargestBallRadius * GameConfig::LargestBallRadius),
				0.0f,
				1.0f);

			const float BuoyancyGravityScale =
				std::lerp(
					GameConfig::MinBuoyancyGravityScale,
					GameConfig::MaxBuoyancyGravityScale,
					SizeRatio) *
				SubmergedRatio;
			const float BuoyancyAcceleration =
				-Settings.GravityAcceleration.y * BuoyancyGravityScale;

			Ball->AddVelocity(FVector(0.0f, BuoyancyAcceleration * SubstepDeltaTime, 0.0f));

			const float WaterDragCoefficient = std::lerp(
				GameConfig::MinWaterDragCoefficient,
				GameConfig::MaxWaterDragCoefficient,
				SizeRatio) *
				SubmergedRatio;
			const FVector DragAcceleration = Ball->Velocity * -WaterDragCoefficient;

			Ball->AddVelocity(DragAcceleration * SubstepDeltaTime);

			Ball->Move(SubstepDeltaTime, Settings.AngularDamping);
		}

		for (int SolverIteration = 0;
			SolverIteration < Settings.SolverIterations;
			++SolverIteration)
		{
			ResolveBallCollisions(Balls, TryMerge);
			ResolveBorderCollisions(Balls);
		}
	}
}

void PhysicsWorld::ResolveBallCollisions(
	std::vector<std::unique_ptr<UBall>>& Balls,
	const FBallMergeHandler& TryMerge) const
{
	int bHasMerged = false;
	int bCheckMerged = false;

	for (std::size_t i = 0; i < Balls.size(); ++i)
	{
		UBall* BallA = Balls[i].get();
		if (!BallA->bHasBeenDropped || BallA->bIsMerging)
		{
			continue;
		}

		for (std::size_t j = i + 1; j < Balls.size(); ++j)
		{
			if (bCheckMerged)
			{
				bHasMerged = true;
				bCheckMerged = false;
			}
			else
			{
				bHasMerged = false;
			}

			UBall* BallB = Balls[j].get();
			if (!BallB->bHasBeenDropped ||
				BallB->bIsMerging ||
				!BallA->IsColliding(BallB))
			{
				continue;
			}

			BallA->bHasCollisionThisFrame = true;
			BallB->bHasCollisionThisFrame = true;
			BallA->bHasTouchedSomething = true;
			BallB->bHasTouchedSomething = true;

			if (TryMerge && TryMerge(*BallA, *BallB))
			{
				Balls.erase(Balls.begin() + static_cast<std::ptrdiff_t>(j));
				if (BallA->Level == static_cast<int>(FruitCatalog::LevelCount))
				{
					Balls.erase(Balls.begin() + static_cast<std::ptrdiff_t>(i));
				}
				bCheckMerged = true;
				i--;
				break;
			}

			if (BallA->bIsMerging || BallB->bIsMerging)
			{
				continue;
			}

			CollisionSolver::ResolveBallCollision(
				*BallA,
				*BallB,
				Settings.Restitution,
				Settings.FrictionCoefficient,
				bHasMerged);
		}
	}
}

void PhysicsWorld::ResolveBorderCollisions(
	std::vector<std::unique_ptr<UBall>>& Balls) const
{
	for (const std::unique_ptr<UBall>& Ball : Balls)
	{
		if (!Ball->bHasBeenDropped || Ball->bIsMerging)
		{
			continue;
		}

		if (Ball->Location.x - Ball->Radius < Settings.LeftBorder)
		{
			Ball->Location.x = Settings.LeftBorder + Ball->Radius;
			CollisionSolver::ResolveBorderContact(
				*Ball,
				FVector(1.0f, 0.0f, 0.0f),
				Settings.Restitution,
				Settings.FrictionCoefficient);
		}
		else if (Ball->Location.x + Ball->Radius > Settings.RightBorder)
		{
			Ball->Location.x = Settings.RightBorder - Ball->Radius;
			CollisionSolver::ResolveBorderContact(
				*Ball,
				FVector(-1.0f, 0.0f, 0.0f),
				Settings.Restitution,
				Settings.FrictionCoefficient);
		}

		if (Ball->Location.y - Ball->Radius < Settings.TopBorder)
		{
			Ball->Location.y = Settings.TopBorder + Ball->Radius;
			Ball->bHasTouchedSomething = true;
			CollisionSolver::ResolveBorderContact(
				*Ball,
				FVector(0.0f, 1.0f, 0.0f),
				Settings.Restitution,
				Settings.FrictionCoefficient);
		}
		else if (Ball->Location.y + Ball->Radius > Settings.BottomBorder)
		{
			Ball->Location.y = Settings.BottomBorder - Ball->Radius;
			CollisionSolver::ResolveBorderContact(
				*Ball,
				FVector(0.0f, -1.0f, 0.0f),
				Settings.Restitution,
				Settings.FrictionCoefficient);
		}
	}
}
