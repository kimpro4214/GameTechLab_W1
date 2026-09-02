#include "pch.h"
#include "MergeParticleSystem.h"

#include "Game/FruitCatalog.h"

#include <numbers>

namespace
{
    constexpr int DropletCount = 14;
    constexpr float HalfPi = std::numbers::pi_v<float> * 0.5f;
    constexpr float TwoPi = std::numbers::pi_v<float> * 2.0f;

    std::uniform_real_distribution<float> SplashRotation(0.0f, TwoPi);
    std::uniform_real_distribution<float> AngleJitter(-0.18f, 0.18f);
    std::uniform_real_distribution<float> SpeedDistribution(0.35f, 0.85f);
    std::uniform_real_distribution<float> LifetimeDistribution(0.30f, 0.55f);
    std::uniform_real_distribution<float> SizeDistribution(0.75f, 1.25f);
    std::uniform_real_distribution<float> AspectDistribution(1.5f, 2.5f);
}

void MergeParticleSystem::EmitMerge(FVector MergePosition, int MergeLevel)
{
    const float FruitRadius = FruitCatalog::GetRadius(MergeLevel);
    const float BaseScale = std::clamp(
        FruitRadius * 0.12f,
        0.008f,
        0.035f);

    const FVector Color = FruitCatalog::GetColor(MergeLevel);

    FMergeParticle Splash{};
    Splash.Type = EMergeParticleType::Splash;

    Splash.Position = MergePosition;
    Splash.Color = Color;
    Splash.Lifetime = 0.5f;
    Splash.StartScaleX = FruitRadius * 0.6f;
    Splash.StartScaleY = FruitRadius * 0.6f;
    Splash.EndScaleX = FruitRadius * 2.0f;
    Splash.EndScaleY = FruitRadius * 2.0f;
    Splash.Rotation = SplashRotation(RandomEngine);

    Particles.push_back(Splash);

    for (int Index = 0; Index < DropletCount; ++Index)
    {
        const float Angle =
            (TwoPi * static_cast<float>(Index) / DropletCount) +
            AngleJitter(RandomEngine);

        const FVector Direction(cosf(Angle), sinf(Angle), 0.0f);

        FMergeParticle Droplet{};
        Droplet.Type = EMergeParticleType::Droplet;

        Droplet.Position = MergePosition;
        Droplet.Velocity = Direction * SpeedDistribution(RandomEngine);
        Droplet.Color = Color;

        Droplet.Age = 0.0f;
        Droplet.Lifetime = LifetimeDistribution(RandomEngine);
        Droplet.StartScaleX = Droplet.EndScaleX = BaseScale * SizeDistribution(RandomEngine);
        Droplet.StartScaleY = Droplet.EndScaleY = Droplet.StartScaleX * AspectDistribution(RandomEngine);

        Droplet.Rotation = Angle - HalfPi;

        Particles.push_back(Droplet);
    }
}

void MergeParticleSystem::Update(float DeltaTime)
{
	for (auto& Particle : Particles)
	{
        if (Particle.Type == EMergeParticleType::Droplet)
        {
            Particle.Position += Particle.Velocity * DeltaTime;
            Particle.Velocity.y -= 2.5f * DeltaTime;
            Particle.Rotation = std::atan2(Particle.Velocity.y, Particle.Velocity.x) - 1.570796f;
        }

		Particle.Age += DeltaTime;
	}

	std::erase_if(Particles, [](const FMergeParticle& Particle) { return Particle.Age >= Particle.Lifetime; });
}

void MergeParticleSystem::Clear()
{
	Particles.clear();
}
