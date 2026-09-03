#include "pch.h"
#include "Game/GameSession.h"

#include "Game/FruitCatalog.h"
#include "Game/GameConfig.h"

#include "Audio/Audio.h"

#include "Input/GamepadManager.h"

#include <algorithm>

GameSession::GameSession()
	: Physics(FPhysicsWorldSettings{
		GameConfig::GravityAcceleration,
		GameConfig::LeftBorder,
		GameConfig::RightBorder,
		GameConfig::TopBorder,
		GameConfig::BottomBorder,
		GameConfig::Restitution,
		GameConfig::FrictionCoefficient,
		GameConfig::AngularDamping,
		GameConfig::PhysicsSubsteps,
		GameConfig::CollisionSolverIterations})
	, RandomEngine(std::random_device{}())
{
	ResetGameState();
}

void GameSession::Update(float DeltaTime)
{
	UpdateDropCooldown();
	UpdateStoreCooldown();
	ParticleSystem.Update(DeltaTime);
	AnimationSystem.Update(DeltaTime);

	if (bIsMainMenu || bIsGameOver)
	{
		ResetFrameDebugState();
		return;
	}

	UpdateMerges(DeltaTime);
	Physics.Step(
		Balls,
		DeltaTime,
		[this](UBall& BallA, UBall& BallB)
		{
			return TryMergeBalls(BallA, BallB);
		});
	CheckGameOver();
}

void GameSession::StartGame()
{
	ResetGameState();
	ParticleSystem.Clear();
	AnimationSystem.Clear();
	bIsMainMenu = false;
}

void GameSession::RestartGame()
{
	ResetGameState();
	ParticleSystem.Clear();
	AnimationSystem.Clear();
	bIsMainMenu = false;
}

void GameSession::ReturnToMainMenu()
{
	ResetGameState();
	ParticleSystem.Clear();
	AnimationSystem.Clear();
	bIsMainMenu = true;
}

void GameSession::ToggleLargestFruitSpawnForTesting()
{
	bSpawnLargestFruitForTesting = !bSpawnLargestFruitForTesting;
	NextLevel = bSpawnLargestFruitForTesting
		? static_cast<int>(FruitCatalog::LevelCount) - 1
		: RandomSpawnLevel();
}

void GameSession::GamepadMoveCurrentBall(float CurrentMoveValueX)
{
	UBall* CurrentBall = GetCurrentBall();
	if (CurrentBall == nullptr || CurrentBall->bHasBeenDropped)
	{
		return;
	}

	const float MinBallX = GameConfig::LeftBorder + CurrentBall->Radius;
	const float MaxBallX = GameConfig::RightBorder - CurrentBall->Radius;
	CurrentBall->Location.x = std::clamp(CurrentBall->Location.x + CurrentMoveValueX, MinBallX, MaxBallX);
}

void GameSession::MoveCurrentBall(float MouseWorldX)
{
	UBall* CurrentBall = GetCurrentBall();
	if (CurrentBall == nullptr || CurrentBall->bHasBeenDropped)
	{
		return;
	}

	const float MinBallX = GameConfig::LeftBorder + CurrentBall->Radius;
	const float MaxBallX = GameConfig::RightBorder - CurrentBall->Radius;
	CurrentBall->Location.x = std::clamp(MouseWorldX, MinBallX, MaxBallX);
}

bool GameSession::DropCurrentBall()
{
	UBall* CurrentBall = GetCurrentBall();
	if (!bCanDropBall || CurrentBall == nullptr || CurrentBall->bHasBeenDropped)
	{
		return false;
	}

	const float MinBallX = GameConfig::LeftBorder + CurrentBall->Radius;
	const float MaxBallX = GameConfig::RightBorder - CurrentBall->Radius;
	const float MinOffset =
		std::max(-GameConfig::DropHorizontalJitter, MinBallX - CurrentBall->Location.x);
	const float MaxOffset =
		std::min(GameConfig::DropHorizontalJitter, MaxBallX - CurrentBall->Location.x);
	std::uniform_real_distribution<float> DropOffsetDistribution(MinOffset, MaxOffset);
	CurrentBall->Location.x += DropOffsetDistribution(RandomEngine);

	CurrentBall->bHasBeenDropped = true;
	AddWaitingBall();
	Audio::GetInstance().Play("Drop");
	bCanDropBall = false;
	LastDropTime = std::chrono::steady_clock::now();
	return true;
}

void GameSession::SwapCurrentBall()
{
	UBall* CurrentBall = GetCurrentBall();
	if (!bCanStoreBall || CurrentBall == nullptr || CurrentBall->bHasBeenDropped)
	{
		return;
	}

	if (StorageLevel == -1)
	{
		StorageLevel = CurrentBall->Level;
		CurrentBall->SetLevel(NextLevel, FruitCatalog::GetRadius(NextLevel));
		NextLevel = RandomSpawnLevel();
	}
	else
	{
		const int PreviousLevel = CurrentBall->Level;
		CurrentBall->SetLevel(StorageLevel, FruitCatalog::GetRadius(StorageLevel));
		StorageLevel = PreviousLevel;
	}

	CurrentBall->Location = FVector(-0.25f, GameConfig::BallSpawnY, 0.0f),

	bCanStoreBall = false;
	LastStoreTime = std::chrono::steady_clock::now();
	Audio::GetInstance().Play("Store");
}

const UBall* GameSession::GetCurrentBall() const
{
	return Balls.empty() ? nullptr : Balls.back().get();
}

UBall* GameSession::GetCurrentBall()
{
	return Balls.empty() ? nullptr : Balls.back().get();
}

void GameSession::ResetGameState()
{
	PendingMerges.clear();
	Balls.clear();
	TotalScore = 0;
	StorageLevel = -1;
	NextLevel = RandomSpawnLevel();
	bCanDropBall = true;
	bCanStoreBall = true;
	bIsGameOver = false;
	AddWaitingBall();
}

void GameSession::AddWaitingBall()
{
	const int CurrentLevel = NextLevel;
	NextLevel = RandomSpawnLevel();
	Balls.push_back(std::make_unique<UBall>(
		FVector(-0.25f, GameConfig::BallSpawnY, 0.0f),
		FVector(),
		CurrentLevel,
		FruitCatalog::GetRadius(CurrentLevel)));
}

void GameSession::CheckGameOver()
{
	for (const std::unique_ptr<UBall>& Ball : Balls)
	{
		if (Ball->bHasBeenDropped &&
			Ball->bHasTouchedSomething &&
			Ball->Location.y + Ball->Radius >= GameConfig::GameOverLineY)
		{
			bIsGameOver = true;
			return;
		}
	}
}

bool GameSession::TryMergeBalls(UBall& BallA, UBall& BallB)
{
	if (BallA.bIsMerging || BallB.bIsMerging)
	{
		return false;
	}

	const bool bCanMerge =
		BallA.Level == BallB.Level &&
		BallA.Level < static_cast<int>(FruitCatalog::LevelCount) - 1;
	if (!bCanMerge)
	{
		return false;
	}

	UBall* LowerBall = BallA.Location.y < BallB.Location.y ? &BallA : &BallB;
	UBall* UpperBall = LowerBall == &BallA ? &BallB : &BallA;

	LowerBall->bIsMerging = true;
	UpperBall->bIsMerging = true;

	PendingMerges.push_back({ LowerBall, UpperBall, UpperBall->Location, 0.0f });

	return false;
}

void GameSession::UpdateDropCooldown()
{
	if (!bCanDropBall &&
		std::chrono::steady_clock::now() - LastDropTime >= GameConfig::DropCooldown)
	{
		bCanDropBall = true;
	}
}

void GameSession::UpdateStoreCooldown()
{
	if (!bCanStoreBall &&
		std::chrono::steady_clock::now() - LastStoreTime >= GameConfig::StoreCooldown)
	{
		bCanStoreBall = true;
	}
}

void GameSession::ResetFrameDebugState()
{
	for (const std::unique_ptr<UBall>& Ball : Balls)
	{
		Ball->bHasCollisionDebug = false;
		Ball->bHasCollisionThisFrame = false;
	}
}

int GameSession::RandomSpawnLevel()
{
	if (bSpawnLargestFruitForTesting)
	{
		return static_cast<int>(FruitCatalog::LevelCount) - 1;
	}

	std::uniform_int_distribution<int> Distribution(0, FruitCatalog::HighestSpawnLevel);
	return Distribution(RandomEngine);
}

void GameSession::UpdateMerges(float DeltaTime)
{
	constexpr float MergeDuration = 0.2f;

	for (auto It = PendingMerges.begin(); It != PendingMerges.end();) 
	{
		FPendingMerge& Merge = *It;
		Merge.ElpasedTime += DeltaTime;

		const float Alpha = std::clamp(Merge.ElpasedTime / MergeDuration, 0.0f, 1.0f);

		Merge.UpperBall->Location = Merge.UpperStartLocation * (1.0f - Alpha) + Merge.LowerBall->Location * Alpha;

		if (Alpha < 1.0f)
		{
			++It;
			continue;
		}

		const int CurrentLevel = Merge.LowerBall->Level;
		const int NewLevel = CurrentLevel + 1;

		Merge.LowerBall->SetLevel(NewLevel, FruitCatalog::GetRadius(NewLevel));
		AnimationSystem.StartPop(Merge.LowerBall);

		Merge.LowerBall->bIsMerging = false;

		TotalScore += FruitCatalog::GetMergeScore(CurrentLevel);
		ParticleSystem.EmitMerge(Merge.LowerBall->Location, CurrentLevel);
		Audio::GetInstance().Play("Merge");
		GamepadManager::GetInstance().AddVibration();

		UBall* UpperBall = Merge.UpperBall;
		std::erase_if(Balls, [UpperBall](const std::unique_ptr<UBall>& Ball) { return Ball.get() == UpperBall; });

		It = PendingMerges.erase(It);
	}
}
