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
	void GamepadMoveCurrentBall(float CurrentMoveValueX);
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
	bool TryMergeBalls(UBall& BallA, UBall& BallB);
	void UpdateDropCooldown();
	void UpdateStoreCooldown();
	void ResetFrameDebugState();
	int RandomSpawnLevel();
	UBall* GetCurrentBall();

	std::vector<std::unique_ptr<UBall>> Balls;
	PhysicsWorld Physics;
	std::mt19937 RandomEngine;
	std::chrono::steady_clock::time_point LastDropTime{};
	std::chrono::steady_clock::time_point LastStoreTime{};

	MergeParticleSystem ParticleSystem;

	int TotalScore = 0;
	int NextLevel = 0;
	int StorageLevel = -1;
	bool bCanDropBall = true;
	bool bCanStoreBall = true;
	bool bIsGameOver = false;
	bool bIsMainMenu = true;

	struct FPendingMerge
	{
		UBall* LowerBall = nullptr;
		UBall* UpperBall = nullptr;
		FVector UpperStartLocation;
		float ElpasedTime = 0.0f;
	};

	void UpdateMerges(float DeltaTime);
	std::vector<FPendingMerge> PendingMerges;
};
