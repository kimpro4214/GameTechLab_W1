#include "pch.h"
#include "MergeParticleSystem.h"

#include "Game/FruitCatalog.h"

#include <numbers>

namespace
{
    constexpr int DropletCount = 14;
    constexpr float HalfPi = std::numbers::pi_v<float> * 0.5f;
    constexpr float TwoPi = std::numbers::pi_v<float> * 2.0f;

    std::uniform_real_distribution<float> AngleDistribution(0.0f, TwoPi);
    std::uniform_real_distribution<float> AngleJitter(-0.18f, 0.18f);
    std::uniform_real_distribution<float> SpeedDistribution(0.35f, 0.85f);
    std::uniform_real_distribution<float> LifetimeDistribution(0.30f, 0.55f);
    std::uniform_real_distribution<float> SizeDistribution(2.0f, 3.5f);
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
    Splash.Lifetime = 0.22f;
    Splash.StartScaleX = FruitRadius * 1.2f;
    Splash.StartScaleY = FruitRadius * 1.2f;
    Splash.EndScaleX = FruitRadius * 3.0f;
    Splash.EndScaleY = FruitRadius * 3.0f;
    Splash.Rotation = AngleDistribution(RandomEngine);

    Particles.push_back(Splash);

    FMergeParticle Flash{};
    Flash.Type = EMergeParticleType::Flash;
    Flash.Position = MergePosition;
    Flash.Color = FVector(1.0f, 1.0f, 1.0f);
    Flash.Lifetime = 0.12f;
    Flash.StartScaleX = FruitRadius * 0.35f;
    Flash.StartScaleY = FruitRadius * 0.35f;
    Flash.EndScaleX = FruitRadius * 1.35f;
    Flash.EndScaleY = FruitRadius * 1.35f;

    Particles.push_back(Flash);

    for (int Index = 0; Index < DropletCount; ++Index)
    {
        const float Angle = AngleDistribution(RandomEngine);

        const FVector Direction(cosf(Angle), sinf(Angle), 0.0f);

        FMergeParticle Droplet{};
        Droplet.Type = EMergeParticleType::Droplet;

        Droplet.Position = MergePosition + Direction * FruitRadius * 0.18f;
        Droplet.Velocity = Direction * SpeedDistribution(RandomEngine);
        Droplet.Color = Color;

        Droplet.Age = 0.0f;
        Droplet.Lifetime = LifetimeDistribution(RandomEngine);
        Droplet.StartScaleX = BaseScale * SizeDistribution(RandomEngine);
        Droplet.EndScaleX = Droplet.StartScaleX * 0.25f;
        Droplet.StartScaleY = Droplet.StartScaleX * AspectDistribution(RandomEngine);
		Droplet.EndScaleY = Droplet.StartScaleY * 0.25f;

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
