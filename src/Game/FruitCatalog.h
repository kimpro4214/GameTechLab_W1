#pragma once

#include "FVector.h"

#include <cstddef>

namespace FruitCatalog
{
	inline constexpr std::size_t LevelCount = 11;
	inline constexpr int HighestSpawnLevel = 4;

	float GetRadius(int Level);
	int GetMergeScore(int Level);
	bool IsValidLevel(int Level);
	FVector GetColor(int Level);
}
