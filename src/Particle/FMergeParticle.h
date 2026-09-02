#pragma once

#include "FVector.h"

enum class EMergeParticleType : std::uint8_t
{
	Droplet,
	Splash,
};

struct FMergeParticle
{
	FVector Position;
	FVector Velocity;
	FVector Color;

	float Age = 0.0f;
	float Lifetime = 1.0f;
	float StartScaleX = 1.0f;
	float StartScaleY = 1.0f;
	float EndScaleX = 1.0f;
	float EndScaleY = 1.0f;
	float Rotation = 0.0f;

	EMergeParticleType Type;
};
