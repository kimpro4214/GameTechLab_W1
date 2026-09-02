#pragma once

#include "Particle/FMergeParticle.h"

#include <random>
#include <vector>

class MergeParticleSystem
{
public:
	void EmitMerge(FVector MergePosition, int MergeLevel);
	void Update(float DeltaTime);
	void Clear();

	const std::vector<FMergeParticle>& GetParticles() const { return Particles; };

private:
	std::vector<FMergeParticle> Particles;
	std::mt19937 RandomEngine{ std::random_device{}() };
};
