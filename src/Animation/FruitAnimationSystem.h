#pragma once

#include <unordered_map>

class UBall;

class FruitAnimationSystem
{
public:
	void StartPop(const UBall* Ball);
	void Update(float DeltaTime);
	void Remove(const UBall* Ball);
	void Clear();

	float GetScale(const UBall* Ball) const;

private:
	struct FPopState
	{
		float ElapsedTime = 0.0f;
	};

	std::unordered_map<const UBall*, FPopState> PopStates;
};
