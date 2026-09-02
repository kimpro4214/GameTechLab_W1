#pragma once

#include "FVector.h"

struct FMergeParticle
{
	FVector Position;
	FVector Velocity;
	FVector Color;

	float Age = 0.0f;
	float Lifetime = 0.45f;
	float ScaleX = 0.015f;
	float ScaleY = 0.03f;
	float Rotation = 0.0f;
};
