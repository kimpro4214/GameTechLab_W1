#include "pch.h"
#include "Game/FruitCatalog.h"

#include <algorithm>

namespace
{
	constexpr float BallSizes[FruitCatalog::LevelCount] = {
		0.05f, 0.07f, 0.09f, 0.11f, 0.13f, 0.16f,
		0.19f, 0.22f, 0.25f, 0.30f, 0.40f
	};

	constexpr int ScoreList[FruitCatalog::LevelCount] = {
		1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 66
	};

	int ClampLevel(int Level)
	{
		return std::clamp(Level, 0, static_cast<int>(FruitCatalog::LevelCount) - 1);
	}
}

float FruitCatalog::GetRadius(int Level)
{
	return BallSizes[ClampLevel(Level)];
}

int FruitCatalog::GetMergeScore(int Level)
{
	return ScoreList[ClampLevel(Level)];
}

bool FruitCatalog::IsValidLevel(int Level)
{
	return Level >= 0 && Level < static_cast<int>(LevelCount);
}
