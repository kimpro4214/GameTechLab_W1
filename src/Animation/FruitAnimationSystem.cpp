#include "pch.h"
#include "FruitAnimationSystem.h"

void FruitAnimationSystem::StartPop(const UBall* Ball)
{
	PopStates[Ball] = {};
}

void FruitAnimationSystem::Update(float DeltaTime)
{
	for (auto It = PopStates.begin(); It != PopStates.end();)
	{
		It->second.ElapsedTime += DeltaTime;

		if (It->second.ElapsedTime >= 0.22f)
		{
			It = PopStates.erase(It);
		}
		else
		{
			It++;
		}
	}
}

void FruitAnimationSystem::Remove(const UBall* Ball)
{
	const auto It = PopStates.find(Ball);
	if (It != PopStates.end())
	{
		PopStates.erase(It);
	}
}

void FruitAnimationSystem::Clear()
{
	PopStates.clear();
}

float FruitAnimationSystem::GetScale(const UBall* Ball) const
{
	const auto It = PopStates.find(Ball);
	if (It == PopStates.end())
	{
		return 1.0f;
	}

	const float T = std::clamp(It->second.ElapsedTime / 0.22f, 0.0f, 1.0f);

	if (T < 0.45f)
	{
		return std::lerp(0.5f, 1.18f, T / 0.45f);
	}

	if (T < 0.75f)
	{
		return std::lerp(1.18f, 0.94f, (T - 0.45f) / 0.3f);
	}

	return std::lerp(0.94f, 1.0f, (T - 0.75f) / 0.25f);
}
