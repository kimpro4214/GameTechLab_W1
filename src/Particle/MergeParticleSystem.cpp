#include "pch.h"
#include "MergeParticleSystem.h"

#include "Game/FruitCatalog.h"

#include <numbers>

namespace
{
    constexpr int DropletCount = 14;
    constexpr float HalfPi = std::numbers::pi_v<float> * 0.5f;
    constexpr float TwoPi = std::numbers::pi_v<float> * 2.0f;

    FVector GetMergeParticleColor(int MergeLevel)
    {
        const FVector Colors[] = {
            { 0.85f, 0.49f, 0.13f },
            { 0.87f, 0.80f, 0.18f },
            { 0.62f, 0.80f, 0.19f },
            { 0.19f, 0.72f, 0.35f },
            { 0.17f, 0.74f, 0.76f },
            { 0.23f, 0.56f, 0.84f },
            { 0.34f, 0.35f, 0.82f },
            { 0.58f, 0.25f, 0.78f },
            { 0.83f, 0.23f, 0.60f },
            { 0.82f, 0.21f, 0.22f },
            { 0.58f, 0.32f, 0.18f },
        };

        const int ColorIndex = std::clamp(
            MergeLevel,
            0,
            static_cast<int>(std::size(Colors)) - 1);

        return Colors[ColorIndex];
    }
}

void MergeParticleSystem::EmitMerge(FVector MergePosition, int MergeLevel)
{
    std::uniform_real_distribution<float> AngleJitter(-0.18f, 0.18f);
    std::uniform_real_distribution<float> SpeedDistribution(0.35f, 0.85f);
    std::uniform_real_distribution<float> LifetimeDistribution(0.30f, 0.55f);
    std::uniform_real_distribution<float> SizeDistribution(0.75f, 1.25f);
    std::uniform_real_distribution<float> AspectDistribution(1.5f, 2.5f);

    const float FruitRadius = FruitCatalog::GetRadius(MergeLevel);
    const float BaseScale = std::clamp(
        FruitRadius * 0.12f,
        0.008f,
        0.035f);

    const FVector Color = GetMergeParticleColor(MergeLevel);

    for (int Index = 0; Index < DropletCount; ++Index)
    {
        const float Angle =
            (TwoPi * static_cast<float>(Index) / DropletCount) +
            AngleJitter(RandomEngine);

        const FVector Direction(cosf(Angle), sinf(Angle), 0.0f);

        FMergeParticle Particle{};
        Particle.Position = MergePosition;
        Particle.Velocity = Direction * SpeedDistribution(RandomEngine);
        Particle.Color = Color;

        Particle.Age = 0.0f;
        Particle.Lifetime = LifetimeDistribution(RandomEngine);
        Particle.ScaleX = BaseScale * SizeDistribution(RandomEngine);
        Particle.ScaleY = Particle.ScaleX * AspectDistribution(RandomEngine);

        Particle.Rotation = Angle - HalfPi;

        Particles.push_back(Particle);
    }
}

void MergeParticleSystem::Update(float DeltaTime)
{
	for (auto& Particle : Particles)
	{
		Particle.Position += Particle.Velocity * DeltaTime;
		Particle.Velocity.y -= 2.5f * DeltaTime;
		Particle.Rotation = std::atan2(Particle.Velocity.y, Particle.Velocity.x) - 1.570796f;
		Particle.Age += DeltaTime;
	}

	std::erase_if(Particles, [](const FMergeParticle& Particle) { return Particle.Age >= Particle.Lifetime; });
}

void MergeParticleSystem::Clear()
{
	Particles.clear();
}
