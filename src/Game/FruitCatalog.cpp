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

	const FVector Colors[] = {
		{ 0.42f, 0.50f, 0.58f }, // 0: Common - 슬레이트
		{ 0.18f, 0.82f, 0.30f }, // 1: Uncommon - 그린
		{ 0.05f, 0.76f, 0.58f }, // 2: Uncommon+ - 에메랄드
		{ 0.08f, 0.48f, 1.00f }, // 3: Rare - 블루
		{ 0.18f, 0.22f, 1.00f }, // 4: Rare+ - 코발트
		{ 0.52f, 0.16f, 1.00f }, // 5: Epic - 바이올렛
		{ 0.95f, 0.10f, 0.78f }, // 6: Epic+ - 마젠타
		{ 1.00f, 0.08f, 0.25f }, // 7: Mythic - 크림슨
		{ 1.00f, 0.34f, 0.05f }, // 8: Legendary - 오렌지
		{ 1.00f, 0.70f, 0.06f }, // 9: Ancient - 골드
		{ 1.00f, 0.82f, 0.48f }, // 10: Transcendent - 웜 펄
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

FVector FruitCatalog::GetColor(int Level)
{
	return Colors[ClampLevel(Level)];
}
