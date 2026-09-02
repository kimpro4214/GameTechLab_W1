#pragma once

#include "Physics/PhysicsWorld.h"
#include "Physics/UBall.h"
#include "Particle/MergeParticleSystem.h"
#include "Particle/FMergeParticle.h"

#include <chrono>
#include <memory>
#include <random>
#include <vector>

class GameSession
{
public:
	GameSession();

	void Update(float DeltaTime);
	void StartGame();
	void RestartGame();
	void ReturnToMainMenu();

	void MoveCurrentBall(float MouseWorldX);
	bool DropCurrentBall();
	void SwapCurrentBall();

	const std::vector<std::unique_ptr<UBall>>& GetBalls() const { return Balls; }
	const UBall* GetCurrentBall() const;
	const std::vector<FMergeParticle>& GetParticles() const { return ParticleSystem.GetParticles(); };
	int GetTotalScore() const { return TotalScore; }
	int GetNextLevel() const { return NextLevel; }
	int GetStorageLevel() const { return StorageLevel; }
	bool CanDropBall() const { return bCanDropBall; }
	bool IsMainMenu() const { return bIsMainMenu; }
	bool IsGameOver() const { return bIsGameOver; }

private:
	void ResetGameState();
	void AddWaitingBall();
	void CheckGameOver();
	bool TryMergeBalls(UBall& BallA, const UBall& BallB);
	void UpdateDropCooldown();
	void ResetFrameDebugState();
	int RandomSpawnLevel();
	UBall* GetCurrentBall();

	std::vector<std::unique_ptr<UBall>> Balls;
	PhysicsWorld Physics;
	std::mt19937 RandomEngine;
	std::chrono::steady_clock::time_point LastDropTime{};

	MergeParticleSystem ParticleSystem;

	int TotalScore = 0;
	int NextLevel = 0;
	int StorageLevel = -1;
	bool bCanDropBall = true;
	bool bIsGameOver = false;
	bool bIsMainMenu = true;
};
