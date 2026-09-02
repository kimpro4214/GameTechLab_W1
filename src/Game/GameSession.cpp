#include "Game/GameSession.h"

#include "Game/FruitCatalog.h"
#include "Game/GameConfig.h"

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
	if (bIsMainMenu || bIsGameOver)
	{
		ResetFrameDebugState();
		return;
	}

	Physics.Step(
		Balls,
		DeltaTime,
		[this](UBall& BallA, const UBall& BallB)
		{
			return TryMergeBalls(BallA, BallB);
		});
	CheckGameOver();
}

void GameSession::StartGame()
{
	ResetGameState();
	bIsMainMenu = false;
}

void GameSession::RestartGame()
{
	ResetGameState();
	bIsMainMenu = false;
}

void GameSession::ReturnToMainMenu()
{
	ResetGameState();
	bIsMainMenu = true;
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
	bCanDropBall = false;
	LastDropTime = std::chrono::steady_clock::now();
	return true;
}

void GameSession::SwapCurrentBall()
{
	UBall* CurrentBall = GetCurrentBall();
	if (CurrentBall == nullptr || CurrentBall->bHasBeenDropped)
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

	CurrentBall->Location = FVector(
		-FruitCatalog::GetRadius(CurrentBall->Level) * 0.5f,
		0.9f,
		0.0f);
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
	Balls.clear();
	TotalScore = 0;
	StorageLevel = -1;
	NextLevel = RandomSpawnLevel();
	bCanDropBall = true;
	bIsGameOver = false;
	AddWaitingBall();
}

void GameSession::AddWaitingBall()
{
	const int CurrentLevel = NextLevel;
	NextLevel = RandomSpawnLevel();
	Balls.push_back(std::make_unique<UBall>(
		FVector(-0.25f, 0.9f, 0.0f),
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

bool GameSession::TryMergeBalls(UBall& BallA, const UBall& BallB)
{
	const bool bCanMerge =
		BallA.Level == BallB.Level &&
		BallA.Level < static_cast<int>(FruitCatalog::LevelCount);
	if (!bCanMerge)
	{
		return false;
	}

	TotalScore += FruitCatalog::GetMergeScore(BallA.Level);
	const int NewLevel = BallA.Level + 1;
	BallA.SetLevel(NewLevel, FruitCatalog::GetRadius(NewLevel));
	return true;
}

void GameSession::UpdateDropCooldown()
{
	if (!bCanDropBall &&
		std::chrono::steady_clock::now() - LastDropTime >= GameConfig::DropCooldown)
	{
		bCanDropBall = true;
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
	std::uniform_int_distribution<int> Distribution(0, FruitCatalog::HighestSpawnLevel);
	return Distribution(RandomEngine);
}
